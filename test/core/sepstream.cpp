#include "inspircd.h"
#include <gtest/gtest.h>

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


