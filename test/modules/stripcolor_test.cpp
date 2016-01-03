/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2016 Adam <Adam@anope.org>
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

class stripcolor : public insp::Test
{
 public:
	void SetUp() override
	{
		insp::Test::SetUp();

		ASSERT_TRUE(inspircd->Modules->Load("stripcolor"));
	}
};

TEST_F(stripcolor, test1)
{
	Module *module = inspircd->Modules->Find("stripcolor");
	ASSERT_TRUE(module != nullptr);
}
