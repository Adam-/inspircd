/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2013 Shawn Smith <shawn@inspircd.org>
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007 Robin Burchell <robin+git@viroteck.net>
 *   Copyright (C) 2007 Dennis Friis <peavey@inspircd.org>
 *   Copyright (C) 2006 Craig Edwards <craigedwards@brainbox.cc>
 *   Copyright (C) 2006 Oliver Lupton <oliverlupton@gmail.com>
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


#include "inspircd.h"
#include "modules/ssl.h"

enum
{
	// From UnrealIRCd.
	ERR_SECUREONLYCHAN = 489,
	ERR_ALLMUSTSSL = 490
};

/** Handle channel mode +z
 */
class SSLMode : public ModeHandler
{
 private:
	UserCertificateAPI& API;

 public:
	SSLMode(Module* Creator, UserCertificateAPI& api)
		: ModeHandler(Creator, "sslonly", 'z', PARAM_NONE, MODETYPE_CHANNEL)
		, API(api)
	{
	}

	ModeAction OnModeChange(User* source, User* dest, Channel* channel, std::string& parameter, bool adding) override
	{
		if (adding)
		{
			if (!channel->IsModeSet(this))
			{
				if (IS_LOCAL(source))
				{
					if (!API)
					{
						source->WriteNumeric(ERR_ALLMUSTSSL, channel->name, "Unable to determine whether all members of the channel are connected via SSL");
						return MODEACTION_DENY;
					}

					const Channel::MemberMap& userlist = channel->GetUsers();
					for (Channel::MemberMap::const_iterator i = userlist.begin(); i != userlist.end(); ++i)
					{
						ssl_cert* cert = API->GetCertificate(i->first);
						if (!cert && !i->first->server->IsULine())
						{
							source->WriteNumeric(ERR_ALLMUSTSSL, channel->name, "all members of the channel must be connected via SSL");
							return MODEACTION_DENY;
						}
					}
				}
				channel->SetMode(this, true);
				return MODEACTION_ALLOW;
			}
			else
			{
				return MODEACTION_DENY;
			}
		}
		else
		{
			if (channel->IsModeSet(this))
			{
				channel->SetMode(this, false);
				return MODEACTION_ALLOW;
			}

			return MODEACTION_DENY;
		}
	}
};

/** Handle user mode +z
*/
class SSLModeUser : public ModeHandler
{
 private:
	UserCertificateAPI& API;

 public:
	SSLModeUser(Module* Creator, UserCertificateAPI& api)
		: ModeHandler(Creator, "sslqueries", 'z', PARAM_NONE, MODETYPE_USER)
		, API(api)
	{
	}

	ModeAction OnModeChange(User* user, User* dest, Channel* channel, std::string& parameter, bool adding) override
	{
		if (adding)
		{
			if (!dest->IsModeSet(this))
			{
				if (!API || !API->GetCertificate(user))
					return MODEACTION_DENY;

				dest->SetMode(this, true);
				return MODEACTION_ALLOW;
			}
		}
		else
		{
			if (dest->IsModeSet(this))
			{
				dest->SetMode(this, false);
				return MODEACTION_ALLOW;
			}
		}

		return MODEACTION_DENY;
	}
};

class ModuleSSLModes : public Module
{
 private:
	UserCertificateAPI api;
	SSLMode sslm;
	SSLModeUser sslquery;

 public:
	ModuleSSLModes()
		: api(this)
		, sslm(this, api)
		, sslquery(this, api)
	{
	}

	ModResult OnUserPreJoin(LocalUser* user, Channel* chan, const std::string& cname, std::string& privs, const std::string& keygiven) override
	{
		if(chan && chan->IsModeSet(sslm))
		{
			if (!api)
			{
				user->WriteNumeric(ERR_SECUREONLYCHAN, cname, "Cannot join channel; unable to determine if you are a SSL user (+z)");
				return MOD_RES_DENY;
			}

			if (!api->GetCertificate(user))
			{
				user->WriteNumeric(ERR_SECUREONLYCHAN, cname, "Cannot join channel; SSL users only (+z)");
				return MOD_RES_DENY;
			}
		}

		return MOD_RES_PASSTHRU;
	}

	ModResult OnUserPreMessage(User* user, const MessageTarget& msgtarget, MessageDetails& details) override
	{
		if (msgtarget.type != MessageTarget::TYPE_USER)
			return MOD_RES_PASSTHRU;

		User* target = msgtarget.Get<User>();

		/* If one or more of the parties involved is a ulined service, we wont stop it. */
		if (user->server->IsULine() || target->server->IsULine())
			return MOD_RES_PASSTHRU;

		/* If the target is +z */
		if (target->IsModeSet(sslquery))
		{
			if (!api || !api->GetCertificate(user))
			{
				/* The sending user is not on an SSL connection */
				user->WriteNumeric(ERR_CANTSENDTOUSER, target->nick, "You are not permitted to send private messages to this user (+z set)");
				return MOD_RES_DENY;
			}
		}
		/* If the user is +z */
		else if (user->IsModeSet(sslquery))
		{
			if (!api || !api->GetCertificate(target))
			{
				user->WriteNumeric(ERR_CANTSENDTOUSER, target->nick, "You must remove usermode 'z' before you are able to send private messages to a non-ssl user.");
				return MOD_RES_DENY;
			}
		}

		return MOD_RES_PASSTHRU;
	}

	ModResult OnCheckBan(User *user, Channel *c, const std::string& mask) override
	{
		if ((mask.length() > 2) && (mask[0] == 'z') && (mask[1] == ':'))
		{
			const std::string fp = api ? api->GetFingerprint(user) : "";
			if (!fp.empty() && InspIRCd::Match(fp, mask.substr(2)))
				return MOD_RES_DENY;
		}
		return MOD_RES_PASSTHRU;
	}

	void On005Numeric(std::map<std::string, std::string>& tokens) override
	{
		tokens["EXTBAN"].push_back('z');
	}

	Version GetVersion() override
	{
		return Version("Provides user and channel mode +z to allow for SSL-only channels, queries and notices.", VF_VENDOR);
	}
};

MODULE_INIT(ModuleSSLModes)
