#include "inspircd.h"
#include <gtest/gtest.h>

class TestSuiteThread : public Thread
{
 public:
	TestSuiteThread() : Thread()
	{
	}

	virtual ~TestSuiteThread()
	{
	}

	virtual void Run()
	{
		while (GetExitFlag() == false)
		{
			//std::cout << "Test suite thread run...\n";
			usleep(50);
		}
	}
};

TEST(threads, test1)
{
	std::string anything;
	ThreadEngine* te = NULL;

	//std::cout << "Creating new ThreadEngine class...\n";
	try
	{
		te = new ThreadEngine;
	}
	catch (...)
	{
		//std::cout << "Creation failed, test failure.\n";
		FAIL();
	}
	//std::cout << "Creation success, type " << te->GetName() << "\n";

	//std::cout << "Allocate: new TestSuiteThread...\n";
	TestSuiteThread* tst = new TestSuiteThread();

	//std::cout << "ThreadEngine::Create on TestSuiteThread...\n";
	try
	{
		try
		{
			te->Start(tst);
		}
		catch (CoreException &ce)
		{
			std::cout << "Failure: " << ce.GetReason() << std::endl;
			FAIL();
		}
	}
	catch (...)
	{
		std::cout << "Failure, unhandled exception\n";
		FAIL();
	}

	usleep(25);

	/* Thread engine auto frees thread on delete */
	//std::cout << "Waiting for thread to exit... " << std::flush;
	delete tst;
	//std::cout << "Done!\n";

	//std::cout << "Delete ThreadEngine... ";
	delete te;
	//std::cout << "Done!\n";
}

