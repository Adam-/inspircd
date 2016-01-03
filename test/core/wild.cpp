#include "inspircd.h"
#include <gtest/gtest.h>

class wild : public testing::Test
{
 protected:
	InspIRCd *inspircd;

	void SetUp() override
	{
		char *argv[] = { "inspircd", "--nofork", "--quiet", NULL };
		inspircd = new InspIRCd(3, argv);
	}

	void TearDown() override
	{
		inspircd->Cleanup();
		delete inspircd;
	}
};

/* Test that x matches y with match() */
#define WCTEST(x, y) ASSERT_TRUE(InspIRCd::Match(x, y, NULL))
/* Test that x does not match y with match() */
#define WCTESTNOT(x, y) ASSERT_FALSE(InspIRCd::Match(x, y, NULL))

/* Test that x matches y with match() and cidr enabled */
#define CIDRTEST(x, y) ASSERT_TRUE(InspIRCd::MatchCIDR(x, y, NULL))
/* Test that x does not match y with match() and cidr enabled */
#define CIDRTESTNOT(x, y) ASSERT_FALSE(InspIRCd::MatchCIDR(x, y, NULL))

TEST_F(wild, test1)
{
	WCTEST("foobar", "*");
	WCTEST("foobar", "foo*");
	WCTEST("foobar", "*bar");
	WCTEST("foobar", "foo??r");
	WCTEST("foobar.test", "fo?bar.*t");
	WCTEST("foobar.test", "fo?bar.t*t");
	WCTEST("foobar.tttt", "fo?bar.**t");
	WCTEST("foobar", "foobar");
	WCTEST("foobar", "foo***bar");
	WCTEST("foobar", "*foo***bar");
	WCTEST("foobar", "**foo***bar");
	WCTEST("foobar", "**foobar*");
	WCTEST("foobar", "**foobar**");
	WCTEST("foobar", "**foobar");
	WCTEST("foobar", "**f?*?ar");
	WCTEST("foobar", "**f?*b?r");
	WCTEST("foofar", "**f?*f*r");
	WCTEST("foofar", "**f?*f*?");
	WCTEST("r", "*");
	WCTEST("", "");
	WCTEST("test@foo.bar.test", "*@*.bar.test");
	WCTEST("test@foo.bar.test", "*test*@*.bar.test");
	WCTEST("test@foo.bar.test", "*@*test");

	WCTEST("a", "*a");
	WCTEST("aa", "*a");
	WCTEST("aaa", "*a");
	WCTEST("aaaa", "*a");
	WCTEST("aaaaa", "*a");
	WCTEST("aaaaaa", "*a");
	WCTEST("aaaaaaa", "*a");
	WCTEST("aaaaaaaa", "*a");
	WCTEST("aaaaaaaaa", "*a");
	WCTEST("aaaaaaaaaa", "*a");
	WCTEST("aaaaaaaaaaa", "*a");

	WCTESTNOT("foobar", "bazqux");
	WCTESTNOT("foobar", "*qux");
	WCTESTNOT("foobar", "foo*x");
	WCTESTNOT("foobar", "baz*");
	WCTESTNOT("foobar", "foo???r");
	WCTESTNOT("foobar", "foobars");
	WCTESTNOT("foobar", "**foobar**h");
	WCTESTNOT("foobar", "**foobar**h*");
	WCTESTNOT("foobar", "**f??*bar?");
	WCTESTNOT("foobar", "");
	WCTESTNOT("", "foobar");
	WCTESTNOT("OperServ", "O");
	WCTESTNOT("O", "OperServ");
	WCTESTNOT("foobar.tst", "fo?bar.*g");
	WCTESTNOT("foobar.test", "fo?bar.*tt");

	CIDRTEST("brain@1.2.3.4", "*@1.2.0.0/16");
	CIDRTEST("brain@1.2.3.4", "*@1.2.3.0/24");
	CIDRTEST("192.168.3.97", "192.168.3.0/24");

	CIDRTESTNOT("brain@1.2.3.4", "x*@1.2.0.0/16");
	CIDRTESTNOT("brain@1.2.3.4", "*@1.3.4.0/24");
	CIDRTESTNOT("1.2.3.4", "1.2.4.0/24");
	CIDRTESTNOT("brain@1.2.3.4", "*@/24");
	CIDRTESTNOT("brain@1.2.3.4", "@1.2.3.4/9");
	CIDRTESTNOT("brain@1.2.3.4", "@");
	CIDRTESTNOT("brain@1.2.3.4", "");
}

