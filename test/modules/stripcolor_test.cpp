#include "inspircd.h"
#include <gtest/gtest.h>

class stripcolor : public testing::Test
{
 protected:
	InspIRCd *inspircd;

	void SetUp() override
	{
		char *argv[] = { "inspircd", "--nofork", "--quiet", NULL };
		inspircd = new InspIRCd(3, argv);

		ASSERT_TRUE(inspircd->Modules->Load("stripcolor"));
	}

	void TearDown() override
	{
		inspircd->Cleanup();
		delete inspircd;
	}
};

TEST_F(stripcolor, test1)
{
	Module *module = inspircd->Modules->Find("stripcolor");
	ASSERT_TRUE(module != nullptr);
}
