#include "inspircd.h"
#include <gtest/gtest.h>

class uid : public testing::Test
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

TEST_F(uid, test1)
{
	const unsigned int UUID_LENGTH = UIDGenerator::UUID_LENGTH;
	UIDGenerator uidgen;
	uidgen.init(ServerInstance->Config->sid);
	std::string first_uid = uidgen.GetUID();

	if (first_uid.length() != UUID_LENGTH)
	{
		std::cout << "GENERATEUID: Generated UID is " << first_uid.length() << " characters long instead of " << UUID_LENGTH-1 << std::endl;
		FAIL();
	}

	if (uidgen.current_uid.c_str()[UUID_LENGTH] != '\0')
	{
		std::cout << "GENERATEUID: The null terminator is missing from the end of current_uid" << std::endl;
		FAIL();
	}

	// The correct UID when generating one for the first time is ...AAAAAA
	std::string correct_uid = ServerInstance->Config->sid + std::string(UUID_LENGTH - 3, 'A');
	if (first_uid != correct_uid)
	{
		std::cout << "GENERATEUID: Generated an invalid first UID: " << first_uid << " instead of " << correct_uid << std::endl;
		FAIL();
	}

	// Set current_uid to be ...Z99999
	uidgen.current_uid[3] = 'Z';
	for (unsigned int i = 4; i < UUID_LENGTH; i++)
		uidgen.current_uid[i] = '9';

	// Store the UID we'll be incrementing so we can display what's wrong later if necessary
	std::string before_increment(uidgen.current_uid);
	std::string generated_uid = uidgen.GetUID();

	// Correct UID after incrementing ...Z99999 is ...0AAAAA
	correct_uid = ServerInstance->Config->sid + "0" + std::string(UUID_LENGTH - 4, 'A');

	if (generated_uid != correct_uid)
	{
		std::cout << "GENERATEUID: Generated an invalid UID after incrementing " << before_increment << ": " << generated_uid << " instead of " << correct_uid << std::endl;
		FAIL();
	}

	// Set current_uid to be ...999999 to see if it rolls over correctly
	for (unsigned int i = 3; i < UUID_LENGTH; i++)
		uidgen.current_uid[i] = '9';

	before_increment.assign(uidgen.current_uid);
	generated_uid = uidgen.GetUID();

	// Correct UID after rolling over is the first UID we've generated (...AAAAAA)
	if (generated_uid != first_uid)
	{
		std::cout << "GENERATEUID: Generated an invalid UID after incrementing " << before_increment << ": " << generated_uid << " instead of " << first_uid << std::endl;
		FAIL();
	}
}

