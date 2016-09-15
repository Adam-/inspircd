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
#include "inspircd.h"
#include "caller.h"
#include <boost/locale.hpp>

/* $LinkerFlags: -lboost_locale */

namespace
{

std::locale locale;

}

class LocaleHashCompare : public HashCompare
{
	const boost::locale::collator<char> &ct;

 public:
	LocaleHashCompare() : ct(std::use_facet<boost::locale::collator<char>>(locale))
	{
	}

	int compare(const std::string& s1, const std::string& s2) override
	{
		std::string c1 = boost::locale::conv::between(s1.data(), s1.data() + s1.length(), locale.name(), "");
		std::string c2 = boost::locale::conv::between(s2.data(), s2.data() + s2.length(), locale.name(), "");

		return ct.compare(boost::locale::collator_base::secondary, c1, c2);
	}

	size_t hash(const std::string &s) override
	{
		return ct.hash(boost::locale::collator_base::secondary, s.data(), s.data() + s.length());
	}
};

class NickHandler : public HandlerBase1<bool, const std::string&>
{
 public:
	bool Call(const std::string& n) override
	{
		if (n.empty() || n.length() > ServerInstance->Config->Limits.NickMax)
			return false;

		/* "0"-"9", "-" can occur anywhere BUT the first char of a nickname */
		if (n[0] == '-' || (n[0] >= '0' && n[0] <= '9'))
			return false;

		try
		{
			boost::locale::conv::to_utf<char>(n.data(), n.data() + n.length(), locale, boost::locale::conv::stop);
		}
		catch (const boost::locale::conv::conversion_error &)
		{
			return false;
		}

		return true;
	}
};

class ModuleUTF8 : public Module
{
	caller1<bool, const std::string&> rememberer;
	NickHandler nh;

 public:
	ModuleUTF8() : rememberer(ServerInstance->IsNick)
	{
		boost::locale::generator gen;
		locale = gen.generate("utf-8");
	}

	~ModuleUTF8()
	{
		ServerInstance->IsNick = rememberer;
	}

	void init() CXX11_OVERRIDE
	{
		ServerInstance->IsNick = &nh;
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides UTF8 nickname support", VF_VENDOR|VF_COMMON);
	}
};

MODULE_INIT(ModuleUTF8)
