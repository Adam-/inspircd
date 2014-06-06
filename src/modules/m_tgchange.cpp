/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2014 Adam <Adam@anope.org>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * tgchange. Detects users messaging many different targets rapidly.
 * Ported from Charybdis's tgchange.c
 */

#include "inspircd.h"

enum
{
	ERR_TARGCHANGE = 707
};

namespace
{
	unsigned TGCHANGE_NUM;   /* how many targets we keep track of */
	unsigned TGCHANGE_REPLY; /* how many reply targets */
}

typedef void Target;

class TGReply
{
	User *user;

 protected:
	std::deque<Target *> reply_targets;

 public:
	TGReply(User *u) : user(u) { }

	void AddReply(User *target)
	{
		if (user == target || ServerInstance->ULine(target->server))
			return;

		std::deque<Target *>::iterator it = std::find(reply_targets.begin(), reply_targets.end(), target);
		if (it != reply_targets.end())
			/* already exists, move to front */
			reply_targets.erase(it);
		else if (reply_targets.size() == TGCHANGE_REPLY)
			/* list growing too large? */
			reply_targets.pop_back();

		reply_targets.push_front(target);
	}
};

class TGInfo : public TGReply
{
	LocalUser *user;

	time_t last;

	std::deque<Target *> targets;

 public:
	TGInfo(LocalUser *u) : TGReply(u), user(u), last(0)
	{
	}

	bool Add(User *target)
	{
		if (user == target || ServerInstance->ULine(target->server))
			return true;

		return AddTarget(target);
	}

	bool Add(Channel *target)
	{
		return AddTarget(target);
	}

 private:
	bool AddTarget(Target *target)
	{
		/* already exists? */
		std::deque<Target *>::iterator it = std::find(targets.begin(), targets.end(), target);
		if (it != targets.end())
		{
			/* yes. move it to the beginning */
			targets.erase(it);
			targets.push_front(target);
			return true;
		}

		/* or as a reply target? */
		it = std::find(reply_targets.begin(), reply_targets.end(), target);
		if (it != reply_targets.end())
		{
			reply_targets.erase(it);
			reply_targets.push_front(target);
			return true;
		}

		/* try to clear some old targets  */
		time_t t = (ServerInstance->Time() - last) / 60;
		if (t > 0)
		{
			/* clear one target per minute */

			size_t sz = targets.size();
			if (static_cast<size_t>(t) > sz)
				sz = 0;
			else
				sz -= t;

			targets.resize(sz);

			last = ServerInstance->Time();
		}

		if (targets.size() == TGCHANGE_NUM)
		{
			/* still no free targets? */
			return false;
		}

		/* add new target */
		targets.push_front(target);

		return true;
	}
};

class TGExtInfo : public SimpleExtItem<TGReply>
{
 public:
	TGExtInfo(const std::string& Key, Module* parent) : SimpleExtItem<TGReply>(Key, parent) { }

	TGReply *get_user(User *user)
	{
		LocalUser *luser = IS_LOCAL(user);
		if (luser)
			return get_user(luser);

		TGReply *t = get(user);
		if (!t)
		{
			t = new TGReply(user);
			set(user, t);
		}
		return t;
	}

	TGInfo *get_user(LocalUser *user)
	{
		TGInfo *t = static_cast<TGInfo *>(get(user));
		if (!t)
		{
			t = new TGInfo(user);
			set(user, t);
		}
		return t;
	}
};

class ModuleTGChange : public Module
{
	TGExtInfo tginfo;

 public:
	ModuleTGChange() : tginfo("tginfo", this)
	{
	}

	void init()
	{
		Implementation eventlist[] = { I_OnRehash, I_OnUserPreMessage, I_OnUserPreNotice, I_OnUserPreInvite };
		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist) / sizeof(Implementation));

		OnRehash(NULL);
	}

	Version GetVersion()
	{
		return Version("", VF_VENDOR);
	}

	void OnRehash(User* user)
	{
		ConfigTag *tag = ServerInstance->Config->ConfValue("tgchange");
		TGCHANGE_NUM = tag->getInt("num", 10);
		TGCHANGE_REPLY = tag->getInt("reply", 5);
	}

	ModResult OnUserPreMessage(User* user, void* dest, int target_type, std::string &text, char status, CUList &exempt_list)
	{
		LocalUser *luser = IS_LOCAL(user);
		if (!luser)
			return MOD_RES_PASSTHRU;

		if (target_type == TYPE_USER)
			return Target(luser, static_cast<User *>(dest));

		if (target_type == TYPE_CHANNEL)
			return Target(luser, static_cast<Channel *>(dest));

		return MOD_RES_PASSTHRU;
	}

	ModResult OnUserPreNotice(User* user, void* dest, int target_type, std::string &text, char status, CUList &exempt_list)
	{
		/* don't count ctcp replies */
		if (text.empty() || text[0] == 1)
			return MOD_RES_PASSTHRU;

		return OnUserPreMessage(user, dest, target_type, text, status, exempt_list);
	}

	ModResult OnUserPreInvite(User* source, User* dest, Channel* channel, time_t timeout)
	{
		LocalUser *luser = IS_LOCAL(source);
		if (!luser)
			return MOD_RES_PASSTHRU;

		return Target(luser, dest);
	}

 private:
	ModResult Target(LocalUser *source, User *dest)
	{
		ModResult m = Target(source, dest, dest->nick);
		if (m != MOD_RES_PASSTHRU)
			return m;

		TGReply *r = tginfo.get_user(dest);
		r->AddReply(source);

		return MOD_RES_PASSTHRU;
	}

	ModResult Target(LocalUser *source, Channel *dest)
	{
		return Target(source, dest, dest->name);
	}

	template<typename T>
	ModResult Target(LocalUser *source, T *target, const std::string &name)
	{
		TGInfo *tg = tginfo.get_user(source);
		if (!tg->Add(target))
		{
			source->WriteNumeric(ERR_TARGCHANGE, "%s %s :Targets changing too fast, message dropped", source->nick.c_str(), name.c_str());
			return MOD_RES_DENY;
		}

		return MOD_RES_PASSTHRU;
	}
};

MODULE_INIT(ModuleTGChange)
