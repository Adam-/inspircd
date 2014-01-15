/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
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


#pragma once

enum
{
	BANCACHE_MISS = -1,
	BANCACHE_NOTCACHED = 0,
	BANCACHE_HIT = 1
};

class CoreExport Membership : public Extensible
{
 public:
	User* const user;
	Channel* const chan;
	// mode list, sorted by prefix rank, higest first
	std::string modes;
	short bancache;
	unsigned int cachev;

	Membership(User* u, Channel* c) : user(u), chan(c), bancache(0), cachev(0) {}

	inline bool hasMode(char m) const
	{
		return modes.find(m) != std::string::npos;
	}

	unsigned int getRank();

	/** Add a prefix character to a user.
	 * Only the core should call this method, usually from
	 * within the mode parser or when the first user joins
	 * the channel (to grant the default privs to them)
	 * @param mh The mode handler of the prefix mode to associate
	 * @param adding True if adding the prefix, false when removing
	 * @return True if a change was made
	 */
	bool SetPrefix(PrefixMode* mh, bool adding);
};

class CoreExport InviteBase
{
 protected:
	InviteList invites;

 public:
	void ClearInvites();

	friend class Invitation;
};

class CoreExport Invitation : public classbase
{
	Invitation(Channel* c, LocalUser* u, time_t timeout) : user(u), chan(c), expiry(timeout) {}

 public:
	LocalUser* const user;
	Channel* const chan;
	time_t expiry;

	~Invitation();
	static void Create(Channel* c, LocalUser* u, time_t timeout);
	static Invitation* Find(Channel* c, LocalUser* u, bool check_expired = true);
};
