#include "pch.h"
#include "CppUnitTest.h"
#include "Shared/Metrics.h"
#ifdef _WIN32
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{

	TEST_CLASS(Metrics_Tests)
	{
	public:
		
		TEST_METHOD(Calculation_getSum)
		{
			// setup
			Metrics::Calculations calc;
			int numbers[4] = { 1,3,6,4 };
			float expectedSum = 14;
			float answer;

			//action
			for (int i = 0; i < sizeof(numbers)/sizeof(int); i++) {
				calc.addPoint(numbers[i]);
			}
			answer = calc.getSum();

			//assert
			Assert::AreEqual(expectedSum, answer);
		}

		TEST_METHOD(Calculation_getAverage)
		{
			// setup
			Metrics::Calculations calc;
			int numbers[4] = { 1,3,6,4 };
			float expected = 3.5;
			float answer;

			//action
			for (int i = 0; i < sizeof(numbers) / sizeof(int); i++) {
				calc.addPoint(numbers[i]);
			}
			answer = calc.getAverage();

			//assert
			Assert::AreEqual(expected, answer);
		}

		TEST_METHOD(Calculation_addPoint)
		{
			// setup
			Metrics::Calculations calc;
			float expected = 1;
			float answer;

			//action
			
			calc.addPoint(1);
			
			answer = calc.getSum();

			//assert
			Assert::AreEqual(expected, answer);
		}
		TEST_METHOD(Timer_Start)
		{
			// setup
			Metrics::Timer timer;
			bool expected = true;
			bool answer;

			//action
			answer = timer.start();
		
			//assert
			Assert::AreEqual(expected, answer);
		}

		#ifdef _WIN32
		TEST_METHOD(getTime_1_Second)
		{
			
			
			// setup
			Metrics::Timer timer;
			float startRange = 1000;
			float endRange = 1200;
			float answer;

			//action
			timer.start();
			Sleep(1000);
			answer = timer.getTime();
			
		
			//assert
			if (answer >= startRange && answer <= endRange) {
				Assert::AreEqual(0, 0);
			}
			else {
				Assert::Fail();
			}
		}
		TEST_METHOD(getTime_3_Second)
		{

			
			// setup
			Metrics::Timer timer;
			float startRange = 3000;
			float endRange = 3200;
			float answer;

			//action
			timer.start();
			Sleep(3000);
			answer = timer.getTime();

			
			//assert
			if (answer >= startRange && answer <= endRange) {
				Assert::AreEqual(0, 0);
			}
			else {
				Assert::Fail();
			}
		}
		#endif
	};

	
}
