/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2012 Shawn Smith <shawn@inspircd.org>
 *   Copyright (C) 2009-2010 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007, 2009 Dennis Friis <peavey@inspircd.org>
 *   Copyright (C) 2006-2008 Robin Burchell <robin+git@viroteck.net>
 *   Copyright (C) 2008 Thomas Stagner <aquanight@inspircd.org>
 *   Copyright (C) 2004-2008 Craig Edwards <craigedwards@brainbox.cc>
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
#include "builtinmodes.h"

ModeHandler::ModeHandler(Module* Creator, const std::string& Name, char modeletter, ParamSpec Params, ModeType type, Class mclass)
	: ServiceProvider(Creator, Name, SERVICE_MODE), m_paramtype(TR_TEXT),
	parameters_taken(Params), mode(modeletter), oper(false),
	list(false), m_type(type), type_id(mclass), levelrequired(HALFOP_VALUE)
{
}

CullResult ModeHandler::cull()
{
	if (ServerInstance->Modes)
		ServerInstance->Modes->DelMode(this);
	return classbase::cull();
}

ModeHandler::~ModeHandler()
{
}

int ModeHandler::GetNumParams(bool adding)
{
	switch (parameters_taken)
	{
		case PARAM_ALWAYS:
			return 1;
		case PARAM_SETONLY:
			return adding ? 1 : 0;
		case PARAM_NONE:
			break;
	}
	return 0;
}

std::string ModeHandler::GetUserParameter(User* user)
{
	return "";
}

ModResult ModeHandler::AccessCheck(User*, Channel*, std::string &, bool)
{
	return MOD_RES_PASSTHRU;
}

ModeAction ModeHandler::OnModeChange(User*, User*, Channel*, std::string&, bool)
{
	return MODEACTION_DENY;
}

void ModeHandler::DisplayList(User*, Channel*)
{
}

void ModeHandler::DisplayEmptyList(User*, Channel*)
{
}

void ModeHandler::OnParameterMissing(User* user, User* dest, Channel* channel)
{
}

bool ModeHandler::ResolveModeConflict(std::string& theirs, const std::string& ours, Channel*)
{
	return (theirs < ours);
}

ModeAction SimpleUserModeHandler::OnModeChange(User* source, User* dest, Channel* channel, std::string &parameter, bool adding)
{
	/* We're either trying to add a mode we already have or
		remove a mode we don't have, deny. */
	if (dest->IsModeSet(this) == adding)
		return MODEACTION_DENY;

	/* adding will be either true or false, depending on if we
		are adding or removing the mode, since we already checked
		to make sure we aren't adding a mode we have or that we
		aren't removing a mode we don't have, we don't have to do any
		other checks here to see if it's true or false, just add or
		remove the mode */
	dest->SetMode(this, adding);

	return MODEACTION_ALLOW;
}


ModeAction SimpleChannelModeHandler::OnModeChange(User* source, User* dest, Channel* channel, std::string &parameter, bool adding)
{
	/* We're either trying to add a mode we already have or
		remove a mode we don't have, deny. */
	if (channel->IsModeSet(this) == adding)
		return MODEACTION_DENY;

	/* adding will be either true or false, depending on if we
		are adding or removing the mode, since we already checked
		to make sure we aren't adding a mode we have or that we
		aren't removing a mode we don't have, we don't have to do any
		other checks here to see if it's true or false, just add or
		remove the mode */
	channel->SetMode(this, adding);

	return MODEACTION_ALLOW;
}

ModeAction ParamChannelModeHandler::OnModeChange(User* source, User* dest, Channel* channel, std::string &parameter, bool adding)
{
	if (adding && !ParamValidate(parameter))
		return MODEACTION_DENY;
	std::string now = channel->GetModeParameter(this);
	if (parameter == now)
		return MODEACTION_DENY;
	return MODEACTION_ALLOW;
}

bool ParamChannelModeHandler::ParamValidate(std::string& parameter)
{
	return true;
}

ModeWatcher::ModeWatcher(Module* Creator, const std::string& modename, ModeType type)
	: mode(modename), m_type(type), creator(Creator)
{
	ServerInstance->Modes->AddModeWatcher(this);
}

ModeWatcher::~ModeWatcher()
{
	ServerInstance->Modes->DelModeWatcher(this);
}

ModeType ModeWatcher::GetModeType()
{
	return m_type;
}

bool ModeWatcher::BeforeMode(User*, User*, Channel*, std::string&, bool)
{
	return true;
}

void ModeWatcher::AfterMode(User*, User*, Channel*, const std::string&, bool)
{
}

void ModeParser::DisplayCurrentModes(User *user, User* targetuser, Channel* targetchannel, const char* text)
{
	if (targetchannel)
	{
		/* Display channel's current mode string */
		user->WriteNumeric(RPL_CHANNELMODEIS, "%s +%s", targetchannel->name.c_str(), targetchannel->ChanModes(targetchannel->HasUser(user)));
		user->WriteNumeric(RPL_CHANNELCREATED, "%s %lu", targetchannel->name.c_str(), (unsigned long)targetchannel->age);
		return;
	}
	else
	{
		if (targetuser == user || user->HasPrivPermission("users/auspex"))
		{
			/* Display user's current mode string */
			user->WriteNumeric(RPL_UMODEIS, ":+%s", targetuser->FormatModes());
			if ((targetuser->IsOper()))
			{
				ModeHandler* snomask = FindMode('s', MODETYPE_USER), * snomaskRemote = FindMode('S', MODETYPE_USER);
				if (user->IsModeSet(snomask))
					user->WriteNumeric(RPL_SNOMASKIS, "%s :Local server notice mask", snomask->GetUserParameter(user).c_str());
				if (user->IsModeSet(snomaskRemote))
					user->WriteNumeric(RPL_SNOMASKIS, "%s :Remote server notice mask", snomaskRemote->GetUserParameter(user).c_str());
			}
			return;
		}
		else
		{
			user->WriteNumeric(ERR_USERSDONTMATCH, ":Can't view modes for other users");
			return;
		}
	}
}

PrefixMode::PrefixMode(Module* Creator, const std::string& Name, char ModeLetter)
	: ModeHandler(Creator, Name, ModeLetter, PARAM_ALWAYS, MODETYPE_CHANNEL, MC_PREFIX)
	, prefix(0), prefixrank(0)
{
	list = true;
	m_paramtype = TR_NICK;
}

ModeAction PrefixMode::OnModeChange(User* source, User*, Channel* chan, std::string& parameter, bool adding)
{
	User* target;
	if (IS_LOCAL(source))
		target = ServerInstance->FindNickOnly(parameter);
	else
		target = ServerInstance->FindNick(parameter);

	if (!target)
	{
		source->WriteNumeric(ERR_NOSUCHNICK, "%s :No such nick/channel", parameter.c_str());
		return MODEACTION_DENY;
	}

	Membership* memb = chan->GetUser(target);
	if (!memb)
		return MODEACTION_DENY;

	parameter = target->nick;
	return (memb->SetPrefix(this, adding) ? MODEACTION_ALLOW : MODEACTION_DENY);
}

ModeAction ModeParser::TryMode(User* user, User* targetuser, Channel* chan, bool adding, const unsigned char modechar,
		std::string &parameter, bool SkipACL)
{
	ModeType type = chan ? MODETYPE_CHANNEL : MODETYPE_USER;

	ModeHandler *mh = FindMode(modechar, type);
	int pcnt = mh->GetNumParams(adding);

	// crop mode parameter size to 250 characters
	if (parameter.length() > 250 && adding)
		parameter = parameter.substr(0, 250);

	ModResult MOD_RESULT;
	FIRST_MOD_RESULT(OnRawMode, MOD_RESULT, (user, chan, modechar, parameter, adding, pcnt));

	if (IS_LOCAL(user) && (MOD_RESULT == MOD_RES_DENY))
		return MODEACTION_DENY;

	if (chan && !SkipACL && (MOD_RESULT != MOD_RES_ALLOW))
	{
		MOD_RESULT = mh->AccessCheck(user, chan, parameter, adding);

		if (MOD_RESULT == MOD_RES_DENY)
			return MODEACTION_DENY;
		if (MOD_RESULT == MOD_RES_PASSTHRU)
		{
			unsigned int neededrank = mh->GetLevelRequired();
			/* Compare our rank on the channel against the rank of the required prefix,
			 * allow if >= ours. Because mIRC and xchat throw a tizz if the modes shown
			 * in NAMES(X) are not in rank order, we know the most powerful mode is listed
			 * first, so we don't need to iterate, we just look up the first instead.
			 */
			unsigned int ourrank = chan->GetPrefixValue(user);
			if (ourrank < neededrank)
			{
				PrefixMode* neededmh = NULL;
				for(char c='A'; c <= 'z'; c++)
				{
					PrefixMode* privmh = FindPrefixMode(c);
					if (privmh && privmh->GetPrefixRank() >= neededrank)
					{
						// this mode is sufficient to allow this action
						if (!neededmh || privmh->GetPrefixRank() < neededmh->GetPrefixRank())
							neededmh = privmh;
					}
				}
				if (neededmh)
					user->WriteNumeric(ERR_CHANOPRIVSNEEDED, "%s :You must have channel %s access or above to %sset channel mode %c",
						chan->name.c_str(), neededmh->name.c_str(), adding ? "" : "un", modechar);
				else
					user->WriteNumeric(ERR_CHANOPRIVSNEEDED, "%s :You cannot %sset channel mode %c",
						chan->name.c_str(), adding ? "" : "un", modechar);
				return MODEACTION_DENY;
			}
		}
	}

	// Ask mode watchers whether this mode change is OK
	std::pair<ModeWatchIter, ModeWatchIter> itpair = modewatchermap.equal_range(mh->name);
	for (ModeWatchIter i = itpair.first; i != itpair.second; ++i)
	{
		ModeWatcher* mw = i->second;
		if (mw->GetModeType() == type)
		{
			if (!mw->BeforeMode(user, targetuser, chan, parameter, adding))
				return MODEACTION_DENY;

			// A module whacked the parameter completely, and there was one. Abort.
			if (pcnt && parameter.empty())
				return MODEACTION_DENY;
		}
	}

	if (IS_LOCAL(user) && !user->IsOper())
	{
		char* disabled = (type == MODETYPE_CHANNEL) ? ServerInstance->Config->DisabledCModes : ServerInstance->Config->DisabledUModes;
		if (disabled[modechar - 'A'])
		{
			user->WriteNumeric(ERR_NOPRIVILEGES, ":Permission Denied - %s mode %c has been locked by the administrator",
				type == MODETYPE_CHANNEL ? "channel" : "user", modechar);
			return MODEACTION_DENY;
		}
	}

	if (adding && IS_LOCAL(user) && mh->NeedsOper() && !user->HasModePermission(modechar, type))
	{
		/* It's an oper only mode, and they don't have access to it. */
		if (user->IsOper())
		{
			user->WriteNumeric(ERR_NOPRIVILEGES, ":Permission Denied - Oper type %s does not have access to set %s mode %c",
					user->oper->name.c_str(), type == MODETYPE_CHANNEL ? "channel" : "user", modechar);
		}
		else
		{
			user->WriteNumeric(ERR_NOPRIVILEGES, ":Permission Denied - Only operators may set %s mode %c",
					type == MODETYPE_CHANNEL ? "channel" : "user", modechar);
		}
		return MODEACTION_DENY;
	}

	/* Call the handler for the mode */
	ModeAction ma = mh->OnModeChange(user, targetuser, chan, parameter, adding);

	if (pcnt && parameter.empty())
		return MODEACTION_DENY;

	if (ma != MODEACTION_ALLOW)
		return ma;

	if ((!mh->IsListMode()) && (mh->GetNumParams(true)) && (chan))
		chan->SetModeParam(mh, (adding ? parameter : ""));

	itpair = modewatchermap.equal_range(mh->name);
	for (ModeWatchIter i = itpair.first; i != itpair.second; ++i)
	{
		ModeWatcher* mw = i->second;
		if (mw->GetModeType() == type)
			mw->AfterMode(user, targetuser, chan, parameter, adding);
	}

	return MODEACTION_ALLOW;
}

void ModeParser::Process(const std::vector<std::string>& parameters, User* user, ModeProcessFlag flags)
{
	const std::string& target = parameters[0];
	Channel* targetchannel = ServerInstance->FindChan(target);
	User* targetuser = NULL;
	if (!targetchannel)
	{
		if (IS_LOCAL(user))
			targetuser = ServerInstance->FindNickOnly(target);
		else
			targetuser = ServerInstance->FindNick(target);
	}
	ModeType type = targetchannel ? MODETYPE_CHANNEL : MODETYPE_USER;

	LastParse.clear();
	LastParseParams.clear();
	LastParseTranslate.clear();

	if ((!targetchannel) && ((!targetuser) || (IS_SERVER(targetuser))))
	{
		user->WriteNumeric(ERR_NOSUCHNICK, "%s :No such nick/channel", target.c_str());
		return;
	}
	if (parameters.size() == 1)
	{
		this->DisplayCurrentModes(user, targetuser, targetchannel, target.c_str());
		return;
	}

	ModResult MOD_RESULT;
	FIRST_MOD_RESULT(OnPreMode, MOD_RESULT, (user, targetuser, targetchannel, parameters));

	bool SkipAccessChecks = false;

	if (!IS_LOCAL(user) || MOD_RESULT == MOD_RES_ALLOW)
		SkipAccessChecks = true;
	else if (MOD_RESULT == MOD_RES_DENY)
		return;

	if (targetuser && !SkipAccessChecks && user != targetuser)
	{
		user->WriteNumeric(ERR_USERSDONTMATCH, ":Can't change mode for other users");
		return;
	}

	std::string mode_sequence = parameters[1];

	std::string output_mode;
	std::ostringstream output_parameters;
	LastParseParams.push_back(output_mode);
	LastParseTranslate.push_back(TR_TEXT);

	bool adding = true;
	char output_pm = '\0'; // current output state, '+' or '-'
	unsigned int param_at = 2;

	for (std::string::const_iterator letter = mode_sequence.begin(); letter != mode_sequence.end(); letter++)
	{
		unsigned char modechar = *letter;
		if (modechar == '+' || modechar == '-')
		{
			adding = (modechar == '+');
			continue;
		}

		ModeHandler *mh = this->FindMode(modechar, type);
		if (!mh)
		{
			/* No mode handler? Unknown mode character then. */
			user->WriteNumeric(type == MODETYPE_CHANNEL ? ERR_UNKNOWNMODE : ERR_UNKNOWNSNOMASK, "%c :is unknown mode char to me", modechar);
			continue;
		}

		std::string parameter;
		int pcnt = mh->GetNumParams(adding);
		if (pcnt && param_at == parameters.size())
		{
			/* No parameter, continue to the next mode */
			mh->OnParameterMissing(user, targetuser, targetchannel);
			continue;
		}
		else if (pcnt)
		{
			parameter = parameters[param_at++];
			/* Make sure the user isn't trying to slip in an invalid parameter */
			if ((parameter.find(':') == 0) || (parameter.rfind(' ') != std::string::npos))
				continue;
			if ((flags & MODE_MERGE) && targetchannel && targetchannel->IsModeSet(mh) && !mh->IsListMode())
			{
				std::string ours = targetchannel->GetModeParameter(mh);
				if (!mh->ResolveModeConflict(parameter, ours, targetchannel))
					/* we won the mode merge, don't apply this mode */
					continue;
			}
		}

		ModeAction ma = TryMode(user, targetuser, targetchannel, adding, modechar, parameter, SkipAccessChecks);

		if (ma != MODEACTION_ALLOW)
			continue;

		char needed_pm = adding ? '+' : '-';
		if (needed_pm != output_pm)
		{
			output_pm = needed_pm;
			output_mode.append(1, output_pm);
		}
		output_mode.append(1, modechar);

		if (pcnt)
		{
			output_parameters << " " << parameter;
			LastParseParams.push_back(parameter);
			LastParseTranslate.push_back(mh->GetTranslateType());
		}

		if ( (output_mode.length() + output_parameters.str().length() > 450)
				|| (output_mode.length() > 100)
				|| (LastParseParams.size() > ServerInstance->Config->Limits.MaxModes))
		{
			/* mode sequence is getting too long */
			break;
		}
	}

	LastParseParams[0] = output_mode;

	if (!output_mode.empty())
	{
		LastParse = targetchannel ? targetchannel->name : targetuser->nick;
		LastParse.append(" ");
		LastParse.append(output_mode);
		LastParse.append(output_parameters.str());

		if (!(flags & MODE_LOCALONLY))
			ServerInstance->PI->SendMode(user, targetuser, targetchannel, LastParseParams, LastParseTranslate);

		if (targetchannel)
			targetchannel->WriteChannel(user, "MODE " + LastParse);
		else
			targetuser->WriteFrom(user, "MODE " + LastParse);

		FOREACH_MOD(OnMode, (user, targetuser, targetchannel, LastParseParams, LastParseTranslate));
	}
	else if (targetchannel && parameters.size() == 2)
	{
		/* Special case for displaying the list for listmodes,
		 * e.g. MODE #chan b, or MODE #chan +b without a parameter
		 */
		this->DisplayListModes(user, targetchannel, mode_sequence);
	}
}

void ModeParser::DisplayListModes(User* user, Channel* chan, std::string &mode_sequence)
{
	seq++;

	for (std::string::const_iterator letter = mode_sequence.begin(); letter != mode_sequence.end(); letter++)
	{
		unsigned char mletter = *letter;
		if (mletter == '+')
			continue;

		/* Ensure the user doesnt request the same mode twice,
		 * so they cant flood themselves off out of idiocy.
		 */
		if (sent[mletter] == seq)
			continue;

		sent[mletter] = seq;

		ModeHandler *mh = this->FindMode(mletter, MODETYPE_CHANNEL);

		if (!mh || !mh->IsListMode())
			return;

		ModResult MOD_RESULT;
		FIRST_MOD_RESULT(OnRawMode, MOD_RESULT, (user, chan, mletter, "", true, 0));
		if (MOD_RESULT == MOD_RES_DENY)
			continue;

		bool display = true;
		if (!user->HasPrivPermission("channels/auspex") && ServerInstance->Config->HideModeLists[mletter] && (chan->GetPrefixValue(user) < HALFOP_VALUE))
		{
			user->WriteNumeric(ERR_CHANOPRIVSNEEDED, "%s :You do not have access to view the +%c list",
				chan->name.c_str(), mletter);
			display = false;
		}

		// Ask mode watchers whether it's OK to show the list
		std::pair<ModeWatchIter, ModeWatchIter> itpair = modewatchermap.equal_range(mh->name);
		for (ModeWatchIter i = itpair.first; i != itpair.second; ++i)
		{
			ModeWatcher* mw = i->second;
			if (mw->GetModeType() == MODETYPE_CHANNEL)
			{
				std::string dummyparam;

				if (!mw->BeforeMode(user, NULL, chan, dummyparam, true))
				{
					// A mode watcher doesn't want us to show the list
					display = false;
					break;
				}
			}
		}

		if (display)
			mh->DisplayList(user, chan);
		else
			mh->DisplayEmptyList(user, chan);
	}
}

void ModeParser::CleanMask(std::string &mask)
{
	std::string::size_type pos_of_pling = mask.find_first_of('!');
	std::string::size_type pos_of_at = mask.find_first_of('@');
	std::string::size_type pos_of_dot = mask.find_first_of('.');
	std::string::size_type pos_of_colons = mask.find("::"); /* Because ipv6 addresses are colon delimited -- double so it treats extban as nick */

	if (mask.length() >= 2 && mask[1] == ':')
		return; // if it's an extban, don't even try guess how it needs to be formed.

	if ((pos_of_pling == std::string::npos) && (pos_of_at == std::string::npos))
	{
		/* Just a nick, or just a host - or clearly ipv6 (starting with :) */
		if ((pos_of_dot == std::string::npos) && (pos_of_colons == std::string::npos) && mask[0] != ':')
		{
			/* It has no '.' in it, it must be a nick. */
			mask.append("!*@*");
		}
		else
		{
			/* Got a dot in it? Has to be a host */
			mask = "*!*@" + mask;
		}
	}
	else if ((pos_of_pling == std::string::npos) && (pos_of_at != std::string::npos))
	{
		/* Has an @ but no !, its a user@host */
		 mask = "*!" + mask;
	}
	else if ((pos_of_pling != std::string::npos) && (pos_of_at == std::string::npos))
	{
		/* Has a ! but no @, it must be a nick!ident */
		mask.append("@*");
	}
}

bool ModeParser::AddMode(ModeHandler* mh)
{
	unsigned char mask = 0;
	unsigned char pos = 0;

	/* Yes, i know, this might let people declare modes like '_' or '^'.
	 * If they do that, thats their problem, and if i ever EVER see an
	 * official InspIRCd developer do that, i'll beat them with a paddle!
	 */
	if ((mh->GetModeChar() < 'A') || (mh->GetModeChar() > 'z'))
		return false;

	/* A mode prefix of ',' is not acceptable, it would fuck up server to server.
	 * A mode prefix of ':' will fuck up both server to server, and client to server.
	 * A mode prefix of '#' will mess up /whois and /privmsg
	 */
	PrefixMode* pm = mh->IsPrefixMode();
	if (pm)
	{
		if ((pm->GetPrefix() > 126) || (pm->GetPrefix() == ',') || (pm->GetPrefix() == ':') || (pm->GetPrefix() == '#'))
			return false;

		if (FindPrefix(pm->GetPrefix()))
			return false;
	}

	mh->GetModeType() == MODETYPE_USER ? mask = MASK_USER : mask = MASK_CHANNEL;
	pos = (mh->GetModeChar()-65) | mask;

	if (modehandlers[pos])
		return false;

	// Everything is fine, add the mode
	modehandlers[pos] = mh;
	if (pm)
		mhlist.prefix.push_back(pm);
	else if (mh->IsListModeBase())
		mhlist.list.push_back(mh->IsListModeBase());

	RecreateModeListFor004Numeric();
	return true;
}

bool ModeParser::DelMode(ModeHandler* mh)
{
	unsigned char mask = 0;
	unsigned char pos = 0;

	if ((mh->GetModeChar() < 'A') || (mh->GetModeChar() > 'z'))
		return false;

	mh->GetModeType() == MODETYPE_USER ? mask = MASK_USER : mask = MASK_CHANNEL;
	pos = (mh->GetModeChar()-65) | mask;

	if (modehandlers[pos] != mh)
		return false;

	/* Note: We can't stack here, as we have modes potentially being removed across many different channels.
	 * To stack here we have to make the algorithm slower. Discuss.
	 */
	switch (mh->GetModeType())
	{
		case MODETYPE_USER:
			for (user_hash::iterator i = ServerInstance->Users->clientlist->begin(); i != ServerInstance->Users->clientlist->end(); )
			{
				User* user = i->second;
				++i;
				mh->RemoveMode(user);
			}
		break;
		case MODETYPE_CHANNEL:
			for (chan_hash::iterator i = ServerInstance->chanlist->begin(); i != ServerInstance->chanlist->end(); )
			{
				// The channel may not be in the hash after RemoveMode(), see m_permchannels
				Channel* chan = i->second;
				++i;

				irc::modestacker stack(false);
				mh->RemoveMode(chan, stack);

				std::vector<std::string> stackresult;
				stackresult.push_back(chan->name);
				while (stack.GetStackedLine(stackresult))
				{
					this->Process(stackresult, ServerInstance->FakeClient, MODE_LOCALONLY);
					stackresult.erase(stackresult.begin() + 1, stackresult.end());
				}
			}
		break;
	}

	modehandlers[pos] = NULL;
	if (mh->IsPrefixMode())
		mhlist.prefix.erase(std::find(mhlist.prefix.begin(), mhlist.prefix.end(), mh->IsPrefixMode()));
	else if (mh->IsListModeBase())
		mhlist.list.erase(std::find(mhlist.list.begin(), mhlist.list.end(), mh->IsListModeBase()));

	RecreateModeListFor004Numeric();
	return true;
}

ModeHandler* ModeParser::FindMode(unsigned const char modeletter, ModeType mt)
{
	unsigned char mask = 0;
	unsigned char pos = 0;

	if ((modeletter < 'A') || (modeletter > 'z'))
		return NULL;

	mt == MODETYPE_USER ? mask = MASK_USER : mask = MASK_CHANNEL;
	pos = (modeletter-65) | mask;

	return modehandlers[pos];
}

PrefixMode* ModeParser::FindPrefixMode(unsigned char modeletter)
{
	ModeHandler* mh = FindMode(modeletter, MODETYPE_CHANNEL);
	if (!mh)
		return NULL;
	return mh->IsPrefixMode();
}

std::string ModeParser::CreateModeList(ModeType mt, bool needparam)
{
	std::string modestr;
	unsigned char mask = ((mt == MODETYPE_CHANNEL) ? MASK_CHANNEL : MASK_USER);

	for (unsigned char mode = 'A'; mode <= 'z'; mode++)
	{
		unsigned char pos = (mode-65) | mask;

		if ((modehandlers[pos]) && ((!needparam) || (modehandlers[pos]->GetNumParams(true))))
			modestr.push_back(mode);
	}

	return modestr;
}

void ModeParser::RecreateModeListFor004Numeric()
{
	Cached004ModeList = CreateModeList(MODETYPE_USER) + " " + CreateModeList(MODETYPE_CHANNEL) + " " + CreateModeList(MODETYPE_CHANNEL, true);
}

PrefixMode* ModeParser::FindPrefix(unsigned const char pfxletter)
{
	const PrefixModeList& list = GetPrefixModes();
	for (PrefixModeList::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		PrefixMode* pm = *i;
		if (pm->GetPrefix() == pfxletter)
			return pm;
	}
	return NULL;
}

std::string ModeParser::GiveModeList(ModeMasks m)
{
	std::string type1;	/* Listmodes EXCEPT those with a prefix */
	std::string type2;	/* Modes that take a param when adding or removing */
	std::string type3;	/* Modes that only take a param when adding */
	std::string type4;	/* Modes that dont take a param */

	for (unsigned char mode = 'A'; mode <= 'z'; mode++)
	{
		unsigned char pos = (mode-65) | m;
		 /* One parameter when adding */
		if (modehandlers[pos])
		{
			if (modehandlers[pos]->GetNumParams(true))
			{
				PrefixMode* pm = modehandlers[pos]->IsPrefixMode();
				if ((modehandlers[pos]->IsListMode()) && ((!pm) || (pm->GetPrefix() == 0)))
				{
					type1 += modehandlers[pos]->GetModeChar();
				}
				else
				{
					/* ... and one parameter when removing */
					if (modehandlers[pos]->GetNumParams(false))
					{
						/* But not a list mode */
						if (!pm)
						{
							type2 += modehandlers[pos]->GetModeChar();
						}
					}
					else
					{
						/* No parameters when removing */
						type3 += modehandlers[pos]->GetModeChar();
					}
				}
			}
			else
			{
				type4 += modehandlers[pos]->GetModeChar();
			}
		}
	}

	return type1 + "," + type2 + "," + type3 + "," + type4;
}

std::string ModeParser::BuildPrefixes(bool lettersAndModes)
{
	std::string mletters;
	std::string mprefixes;
	std::map<int,std::pair<char,char> > prefixes;

	const PrefixModeList& list = GetPrefixModes();
	for (PrefixModeList::const_iterator i = list.begin(); i != list.end(); ++i)
	{
		PrefixMode* pm = *i;
		if (pm->GetPrefix())
			prefixes[pm->GetPrefixRank()] = std::make_pair(pm->GetPrefix(), pm->GetModeChar());
	}

	for(std::map<int,std::pair<char,char> >::reverse_iterator n = prefixes.rbegin(); n != prefixes.rend(); n++)
	{
		mletters = mletters + n->second.first;
		mprefixes = mprefixes + n->second.second;
	}

	return lettersAndModes ? "(" + mprefixes + ")" + mletters : mletters;
}

void ModeParser::AddModeWatcher(ModeWatcher* mw)
{
	modewatchermap.insert(std::make_pair(mw->GetModeName(), mw));
}

bool ModeParser::DelModeWatcher(ModeWatcher* mw)
{
	std::pair<ModeWatchIter, ModeWatchIter> itpair = modewatchermap.equal_range(mw->GetModeName());
	for (ModeWatchIter i = itpair.first; i != itpair.second; ++i)
	{
		if (i->second == mw)
		{
			modewatchermap.erase(i);
			return true;
		}
	}

	return false;
}

void ModeHandler::RemoveMode(User* user)
{
	// Remove the mode if it's set on the user
	if (user->IsModeSet(this->GetModeChar()))
	{
		std::vector<std::string> parameters;
		parameters.push_back(user->nick);
		parameters.push_back("-");
		parameters[1].push_back(this->GetModeChar());
		ServerInstance->Modes->Process(parameters, ServerInstance->FakeClient, ModeParser::MODE_LOCALONLY);
	}
}

void ModeHandler::RemoveMode(Channel* channel, irc::modestacker& stack)
{
	if (channel->IsModeSet(this))
	{
		if (this->GetNumParams(false))
			// Removing this mode requires a parameter
			stack.Push(this->GetModeChar(), channel->GetModeParameter(this));
		else
			stack.Push(this->GetModeChar());
	}
}

void PrefixMode::RemoveMode(Channel* chan, irc::modestacker& stack)
{
	const UserMembList* userlist = chan->GetUsers();
	for (UserMembCIter i = userlist->begin(); i != userlist->end(); ++i)
	{
		if (i->second->hasMode(this->GetModeChar()))
			stack.Push(this->GetModeChar(), i->first->nick);
	}
}

struct builtin_modes
{
	ModeChannelSecret s;
	ModeChannelPrivate p;
	ModeChannelModerated m;
	ModeChannelTopicOps t;

	ModeChannelNoExternal n;
	ModeChannelInviteOnly i;
	ModeChannelKey k;
	ModeChannelLimit l;

	ModeChannelBan b;
	ModeChannelOp o;
	ModeChannelVoice v;

	ModeUserWallops uw;
	ModeUserInvisible ui;
	ModeUserOperator uo;
	ModeUserServerNoticeMask us;
	ModeUserServerNoticeMaskRemote uS;

	void init()
	{
		ServiceProvider* modes[] = { &s, &p, &m, &t, &n, &i, &k, &l, &b, &o, &v,
									 &uw, &ui, &uo, &us, &uS };
		ServerInstance->Modules->AddServices(modes, sizeof(modes)/sizeof(ServiceProvider*));
	}
};

static builtin_modes static_modes;

void ModeParser::InitBuiltinModes()
{
	static_modes.init();
	static_modes.b.DoRehash();
}

ModeParser::ModeParser()
{
	/* Clear mode handler list */
	memset(modehandlers, 0, sizeof(modehandlers));

	seq = 0;
	memset(&sent, 0, sizeof(sent));
}

ModeParser::~ModeParser()
{
}
