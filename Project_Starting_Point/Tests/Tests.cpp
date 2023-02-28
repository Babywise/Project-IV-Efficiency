#include "pch.h"
#include "CppUnitTest.h"
#include "../Shared/Metrics.h"
#include "../Shared/configManager.h"
#include "../Client/Client.cpp"
#ifdef _WIN32
#endif
using namespace std;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Unit_Tests
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
	TEST_CLASS(config_Manager_Tests) {

	public:
		TEST_METHOD(getConfig_exists_int) {
			
			std::string expected = "5";
			configuration::configManager manager("../../Tests/TestConfig.conf");
			Assert::AreEqual(atoi(expected.c_str()), atoi(manager.getConfig("test").c_str()));
		}
		TEST_METHOD(getConfig_exists_float) {

			std::string expected = "5.22";
			configuration::configManager manager("../../Tests/TestConfig.conf");
			Assert::AreEqual(atof(expected.c_str()), atof(manager.getConfig("testfloat").c_str()));
		}
		TEST_METHOD(getConfig_exists_string) {

			std::string expected = "works";
			configuration::configManager manager("../../Tests/TestConfig.conf");
			Assert::AreEqual(expected, manager.getConfig("filetest"));
		}
		TEST_METHOD(getConfig_not_exists_int) {
			std::string answer;
			
			configuration::configManager manager("");
			answer = manager.getConfig("not this one");
			Assert::AreEqual(0, (int)answer.size());
		}

	};

	TEST_CLASS(block_tests)
	{
	public:
		TEST_METHOD(Proper_order) {
			std::vector<std::string> expected = { "hello\n","my\n","name\n","is\n","danny" };

			int i = 0;
			char* input = (char*)malloc(100);
			strcpy_s(input, 40, "hello\nmy\nname\nis\ndanny");
			fileIO::block b(input);
		
			while (b.hasNext()) {
				std::string check = b.getNext();
				string help = expected.at(i);
				Assert::AreEqual(help, check);
				i++;
			}
			Assert::AreEqual(1, 1);
		}

		TEST_METHOD(properLineCount) {
			char* input = (char*)malloc(100);
			strcpy_s(input, 40, "hello\nmy\nname\nis\ndanny");
			fileIO::block b(input);
			while (b.getSize() == -1) {
			}
			Assert::AreEqual(5, b.getSize());
		}
		TEST_METHOD(status_Done) {
			char* input = (char*)malloc(100);
			strcpy_s(input, 40, "hello\nmy\nname\nis\ndanny");
			fileIO::block b(input);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			Assert::AreEqual(2, (int)b.getStatus());
		}
		TEST_METHOD(status_Started) {
			char* input = (char*)malloc(100);
			strcpy_s(input, 40, "hello\nmy\nname\nis\ndanny");
			fileIO::block b(input);
			while (b.getStatus() == 1) {

			}
			Assert::AreEqual(2, (int)b.getStatus());
		}
		TEST_METHOD(has_next_true) {

			char* input = (char*)malloc(100);
			strcpy_s(input,40,"hello\nmy\nname\nis\ndanny");
			fileIO::block b(input);
		
			Assert::AreEqual(true, b.hasNext());
		}
		TEST_METHOD(has_next_false) {
			char* input = (char*)malloc(1);
			input[0] = '\0';
			fileIO::block b(input);
			Assert::AreEqual(false, b.hasNext());
		}
	};

	TEST_CLASS(fileBuffer_Tests)
	{
	public:
		TEST_METHOD(has_next_true) {
			configuration::configManager manager("../../Tests/TestConfig.conf");
			fileIO::fileBuffer buffer("../../Client/DataFile.txt");
			Assert::AreEqual(true, buffer.hasNext());
		}
		TEST_METHOD(has_next_false) {
			fileIO::fileBuffer buffer("testData.txt");
			Assert::AreEqual(false, buffer.hasNext());
		}
		TEST_METHOD(nextLine_test) {
			configuration::configManager manager("../../Tests/TestConfig.conf");
			fileIO::fileBuffer buffer("../../Client/DataFile.txt");
			string answer = buffer.next();
			string expected = "ACCELERATION BODY X,ACCELERATION BODY Y,ACCELERATION BODY Z,TOTAL WEIGHT,PLANE ALTITUDE,ATTITUDE INDICATOR PITCH DEGREES,ATTITUDE INDICATOR BANK DEGREES\r\n";

			Assert::AreEqual(expected, answer);
		}
		TEST_METHOD(getLength) {
			configuration::configManager manager("../../Tests/TestConfig.conf");
			fileIO::fileBuffer buffer("../../Client/DataFile.txt");
			while(buffer.getLineCount() == -1) {
				
			}
			Assert::AreEqual(504, buffer.getLineCount());
		}
	};
	
}

namespace metrics_Testing
{
	TEST_CLASS(IO)
	{
	public:
		/// <summary>
		/// This test method is used to ensure PERF_REQ_IO_001 is met
		/// </summary>
		TEST_METHOD(getFileSize)
		{
			//setup
			configuration::configManager manager("../../Tests/TestConfig.conf");
		
			int maxTime = 2; //2 milliseconds
			int time;
			Metrics::Timer timer;

			//act
			timer.start();

			FILE* f;
			int counter = 0;
			fopen_s(&f, manager.getConfigChar("fileLocation"), "rb");

			fseek(f, 0, SEEK_END);
			long fsize = ftell(f);
			fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

			char* data = (char*)malloc(fsize + 1);
			fread(data, fsize, 1, f);
			fclose(f);

			data[fsize] = 0;

			for (int i = 0; i < fsize + 1; i++) {

				if (data[i] == '\n') {
					counter++; // add each new line character as a count
				}
			}
			counter++; // incremement 1 since the last line wont have a new line character
			Sleep(0.1);
			time = timer.getTime();

			//assert
			if (time < maxTime) {
				Assert::AreEqual(1, 1);
			}
			else {
				Assert::Fail();
			}
		}

		TEST_METHOD(totalTimeToGetLine)
		{
			fileIO::fileBuffer buffer("../../Client/DataFile.txt");
			int countTo = buffer.getLineCount();
			timer.start();
			for (int i = 0; i < countTo; i++) {
				std::string strInput = buffer.next();
			}
			float result = timer.getTime();
			float maxTime = 1000;

			if (result < maxTime) {
				Assert::AreEqual(1, 1);
			}
			else {
				Assert::Fail();
			}
		}
		TEST_METHOD(averageTimeToGetLine)
		{
			Metrics::Calculations calculations;
			fileIO::fileBuffer buffer("../../Client/DataFile.txt");
			int countTo = buffer.getLineCount();
			
			for (int i = 0; i < countTo; i++) {
				timer.start();
				std::string strInput = buffer.next();
				calculations.addPoint(timer.getTime());
			}

			float maxTime = 1;

			if (calculations.getAverage() <= maxTime) {
				Assert::AreEqual(1, 1);
			}
			else {
				Assert::Fail();
			}
		}
		
	};
}
