/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Robin Burchell <robin+git@viroteck.net>
 *   Copyright (C) 2008 Craig Edwards <craigedwards@brainbox.cc>
 *   Copyright (C) 2008 Dennis Friis <peavey@inspircd.org>
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

#include "inspircd_test.h"

#define STREQUALTEST(x, y) ASSERT_EQ(x, y)

TEST(sepstream, DoCommaSepStreamTests)
{
	irc::commasepstream items("this,is,a,comma,stream");
	std::string item;
	int idx = 0;

	while (items.GetToken(item))
	{
		idx++;

		switch (idx)
		{
			case 1:
				STREQUALTEST(item, "this");
				break;
			case 2:
				STREQUALTEST(item, "is");
				break;
			case 3:
				STREQUALTEST(item, "a");
				break;
			case 4:
				STREQUALTEST(item, "comma");
				break;
			case 5:
				STREQUALTEST(item, "stream");
				break;
			default:
				std::cout << "COMMASEPSTREAM: FAILURE: Got an index too many! " << idx << " items\n";
				FAIL();
				break;
		}
	}
}

TEST(sepstream, DoSpaceSepStreamTests)
{
	irc::spacesepstream list("this is a space stream");
	std::string item;
	int idx = 0;

	while (list.GetToken(item))
	{
		idx++;

		switch (idx)
		{
			case 1:
				STREQUALTEST(item, "this");
				break;
			case 2:
				STREQUALTEST(item, "is");
				break;
			case 3:
				STREQUALTEST(item, "a");
				break;
			case 4:
				STREQUALTEST(item, "space");
				break;
			case 5:
				STREQUALTEST(item, "stream");
				break;
			default:
				FAIL();
				break;
		}
	}
}


