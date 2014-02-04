/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007 Robin Burchell <robin+git@viroteck.net>
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

/** Handle /USER. These command handlers can be reloaded by the core,
 * and handle basic RFC1459 commands. Commands within modules work
 * the same way, however, they can be fully unloaded, where these
 * may not.
 */
class CommandUser : public SplitCommand
{
 public:
	/** Constructor for user.
	 */
	CommandUser ( Module* parent) : SplitCommand(parent,"USER",4,4) { works_before_reg = true; Penalty = 0; syntax = "<username> <localhost> <remotehost> <GECOS>"; }
	/** Handle command.
	 * @param parameters The parameters to the comamnd
	 * @param pcnt The number of parameters passed to teh command
	 * @param user The user issuing the command
	 * @return A value from CmdResult to indicate command success or failure.
	 */
	CmdResult HandleLocal(const std::vector<std::string>& parameters, LocalUser *user);
};

CmdResult CommandUser::HandleLocal(const std::vector<std::string>& parameters, LocalUser *user)
{
	/* A user may only send the USER command once */
	if (user->registered & REG_USER)
	{
		user->WriteNumeric(ERR_ALREADYREGISTERED, ":You may not reregister");
		return CMD_FAILURE;
	}

	if (!ServerInstance->IsIdent(parameters[0]))
	{
		/*
		 * RFC says we must use this numeric, so we do. Let's make it a little more nub friendly though. :)
		 *  -- Craig, and then w00t.
		 */
		user->WriteNumeric(ERR_NEEDMOREPARAMS, "USER :Your username is not valid");
		return CMD_FAILURE;
	}

	user->ChangeIdent(parameters[0]);
	user->fullname.assign(parameters[3].empty() ? "No info" : parameters[3], 0, ServerInstance->Config->Limits.MaxGecos);
	user->registered |= REG_USER;

	/* parameters 2 and 3 are local and remote hosts, and are ignored */
	if (user->registered == REG_NICKUSER)
	{
		ModResult MOD_RESULT;

		/* user is registered now, bit 0 = USER command, bit 1 = sent a NICK command */
		FIRST_MOD_RESULT(OnUserRegister, MOD_RESULT, (user));
		if (MOD_RESULT == MOD_RES_DENY)
			return CMD_FAILURE;

		ServerInstance->AtomicActions.AddAction(&user->registration);
	}

	return CMD_SUCCESS;
}

COMMAND_INIT(CommandUser)
