/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009-2010 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2008-2009 Robin Burchell <robin+git@viroteck.net>
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
#include "listmode.h"
#include <fstream>


/** Handles the +P channel mode
 */
class PermChannel : public ModeHandler
{
 public:
	PermChannel(Module* Creator)
		: ModeHandler(Creator, "permanent", 'P', PARAM_NONE, MODETYPE_CHANNEL)
	{
		oper = true;
	}

	ModeAction OnModeChange(User* source, User* dest, Channel* channel, std::string& parameter, bool adding)
	{
		if (adding == channel->IsModeSet(this))
			return MODEACTION_DENY;

		channel->SetMode(this, adding);
		if (!adding)
			channel->CheckDestroy();

		return MODEACTION_ALLOW;
	}
};

// Not in a class due to circular dependancy hell.
static std::string permchannelsconf;
static bool WriteDatabase(PermChannel& permchanmode, Module* mod, bool save_listmodes)
{
	ChanModeReference ban(mod, "ban");
	/*
	 * We need to perform an atomic write so as not to fuck things up.
	 * So, let's write to a temporary file, flush it, then rename the file..
	 *     -- w00t
	 */

	// If the user has not specified a configuration file then we don't write one.
	if (permchannelsconf.empty())
		return true;

	std::string permchannelsnewconf = permchannelsconf + ".tmp";
	std::ofstream stream(permchannelsnewconf.c_str());
	if (!stream.is_open())
	{
		ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Cannot create database! %s (%d)", strerror(errno), errno);
		SnomaskManager::Write(SNO_LOCAL, SnomaskManager::announcement, "database: cannot create new db: %s (%d)", strerror(errno), errno);
		return false;
	}

	stream << "# This file is automatically generated by m_permchannels. Any changes will be overwritten." << std::endl
		<< "<config format=\"xml\">" << std::endl;

	for (chan_hash::const_iterator i = ServerInstance->chanlist->begin(); i != ServerInstance->chanlist->end(); i++)
	{
		Channel* chan = i->second;
		if (!chan->IsModeSet(permchanmode))
			continue;

		std::string chanmodes = chan->ChanModes(true);
		if (save_listmodes)
		{
			std::string modes;
			std::string params;

			const ModeParser::ListModeList& listmodes = ServerInstance->Modes->GetListModes();
			for (ModeParser::ListModeList::const_iterator j = listmodes.begin(); j != listmodes.end(); ++j)
			{
				ListModeBase* lm = *j;
				ListModeBase::ModeList* list = lm->GetList(chan);
				if (!list || list->empty())
					continue;

				size_t n = 0;
				// Append the parameters
				for (ListModeBase::ModeList::const_iterator k = list->begin(); k != list->end(); ++k, n++)
				{
					params += k->mask;
					params += ' ';
				}

				// Append the mode letters (for example "IIII", "gg")
				modes.append(n, lm->GetModeChar());
			}

			if (!params.empty())
			{
				// Remove the last space
				params.erase(params.end()-1);

				// If there is at least a space in chanmodes (that is, a non-listmode has a parameter)
				// insert the listmode mode letters before the space. Otherwise just append them.
				std::string::size_type p = chanmodes.find(' ');
				if (p == std::string::npos)
					chanmodes += modes;
				else
					chanmodes.insert(p, modes);

				// Append the listmode parameters (the masks themselves)
				chanmodes += ' ';
				chanmodes += params;
			}
		}

		stream << "<permchannels channel=\"" << ServerConfig::Escape(chan->name)
			<< "\" ts=\"" << chan->age
			<< "\" topic=\"" << ServerConfig::Escape(chan->topic)
			<< "\" topicts=\"" << chan->topicset
			<< "\" topicsetby=\"" << ServerConfig::Escape(chan->setby)
			<< "\" modes=\"" << ServerConfig::Escape(chanmodes)
			<< "\">" << std::endl;
	}

	if (stream.fail())
	{
		ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Cannot write to new database! %s (%d)", strerror(errno), errno);
		SnomaskManager::Write(SNO_LOCAL, SnomaskManager::announcement, "database: cannot write to new db: %s (%d)", strerror(errno), errno);
		return false;
	}
	stream.close();

#ifdef _WIN32
	if (remove(permchannelsconf.c_str()))
	{
		ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Cannot remove old database! %s (%d)", strerror(errno), errno);
		SnomaskManager::Write(SNO_LOCAL, SnomaskManager::announcement, "database: cannot remove old database: %s (%d)", strerror(errno), errno);
		return false;
	}
#endif
	// Use rename to move temporary to new db - this is guarenteed not to fuck up, even in case of a crash.
	if (rename(permchannelsnewconf.c_str(), permchannelsconf.c_str()) < 0)
	{
		ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Cannot move new to old database! %s (%d)", strerror(errno), errno);
		SnomaskManager::Write(SNO_LOCAL, SnomaskManager::announcement, "database: cannot replace old with new db: %s (%d)", strerror(errno), errno);
		return false;
	}

	return true;
}

class ModulePermanentChannels : public Module
{
	PermChannel p;
	bool dirty;
	bool loaded;
	bool save_listmodes;
public:

	ModulePermanentChannels()
		: p(this), dirty(false), loaded(false)
	{
	}

	CullResult cull()
	{
		/*
		 * DelMode can't remove the +P mode on empty channels, or it will break
		 * merging modes with remote servers. Remove the empty channels now as
		 * we know this is not the case.
		 */
		chan_hash::iterator iter = ServerInstance->chanlist->begin();
		while (iter != ServerInstance->chanlist->end())
		{
			Channel* c = iter->second;
			if (c->GetUserCounter() == 0)
			{
				chan_hash::iterator at = iter;
				iter++;
				FOREACH_MOD(OnChannelDelete, (c));
				ServerInstance->chanlist->erase(at);
				ServerInstance->GlobalCulls.AddItem(c);
			}
			else
				iter++;
		}
		ServerInstance->Modes->DelMode(&p);
		return Module::cull();
	}

	void ReadConfig(ConfigStatus& status) CXX11_OVERRIDE
	{
		ConfigTag* tag = ServerInstance->Config->ConfValue("permchanneldb");
		permchannelsconf = tag->getString("filename");
		save_listmodes = tag->getBool("listmodes");
	}

	void LoadDatabase()
	{
		/*
		 * Process config-defined list of permanent channels.
		 * -- w00t
		 */
		ConfigTagList permchannels = ServerInstance->Config->ConfTags("permchannels");
		for (ConfigIter i = permchannels.first; i != permchannels.second; ++i)
		{
			ConfigTag* tag = i->second;
			std::string channel = tag->getString("channel");
			std::string modes = tag->getString("modes");

			if ((channel.empty()) || (channel.length() > ServerInstance->Config->Limits.ChanMax))
			{
				ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Ignoring permchannels tag with empty or too long channel name (\"" + channel + "\")");
				continue;
			}

			Channel *c = ServerInstance->FindChan(channel);

			if (!c)
			{
				time_t TS = tag->getInt("ts", ServerInstance->Time(), 1);
				c = new Channel(channel, TS);

				unsigned int topicset = tag->getInt("topicts");
				c->topic = tag->getString("topic");

				if ((topicset != 0) || (!c->topic.empty()))
				{
					if (topicset == 0)
						topicset = ServerInstance->Time();
					c->topicset = topicset;
					c->setby = tag->getString("topicsetby");
					if (c->setby.empty())
						c->setby = ServerInstance->Config->ServerName;
				}

				ServerInstance->Logs->Log(MODNAME, LOG_DEBUG, "Added %s with topic %s", channel.c_str(), c->topic.c_str());

				if (modes.empty())
					continue;

				irc::spacesepstream list(modes);
				std::string modeseq;
				std::string par;

				list.GetToken(modeseq);

				// XXX bleh, should we pass this to the mode parser instead? ugly. --w00t
				for (std::string::iterator n = modeseq.begin(); n != modeseq.end(); ++n)
				{
					ModeHandler* mode = ServerInstance->Modes->FindMode(*n, MODETYPE_CHANNEL);
					if (mode)
					{
						if (mode->GetNumParams(true))
							list.GetToken(par);
						else
							par.clear();

						mode->OnModeChange(ServerInstance->FakeClient, ServerInstance->FakeClient, c, par, true);
					}
				}
			}
		}
	}

	ModResult OnRawMode(User* user, Channel* chan, ModeHandler* mh, const std::string& param, bool adding) CXX11_OVERRIDE
	{
		if (chan && (chan->IsModeSet(p) || mh == &p))
			dirty = true;

		return MOD_RES_PASSTHRU;
	}

	void OnPostTopicChange(User*, Channel *c, const std::string&) CXX11_OVERRIDE
	{
		if (c->IsModeSet(p))
			dirty = true;
	}

	void OnBackgroundTimer(time_t) CXX11_OVERRIDE
	{
		if (dirty)
			WriteDatabase(p, this, save_listmodes);
		dirty = false;
	}

	void Prioritize()
	{
		// XXX: Load the DB here because the order in which modules are init()ed at boot is
		// alphabetical, this means we must wait until all modules have done their init()
		// to be able to set the modes they provide (e.g.: m_stripcolor is inited after us)
		// Prioritize() is called after all module initialization is complete, consequently
		// all modes are available now
		if (loaded)
			return;

		loaded = true;

		// Load only when there are no linked servers - we set the TS of the channels we
		// create to the current time, this can lead to desync because spanningtree has
		// no way of knowing what we do
		ProtocolInterface::ServerList serverlist;
		ServerInstance->PI->GetServerList(serverlist);
		if (serverlist.size() < 2)
		{
			try
			{
				LoadDatabase();
			}
			catch (CoreException& e)
			{
				ServerInstance->Logs->Log(MODNAME, LOG_DEFAULT, "Error loading permchannels database: " + std::string(e.GetReason()));
			}
		}
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides support for channel mode +P to provide permanent channels",VF_VENDOR);
	}

	ModResult OnChannelPreDelete(Channel *c) CXX11_OVERRIDE
	{
		if (c->IsModeSet(p))
			return MOD_RES_DENY;

		return MOD_RES_PASSTHRU;
	}
};

MODULE_INIT(ModulePermanentChannels)
