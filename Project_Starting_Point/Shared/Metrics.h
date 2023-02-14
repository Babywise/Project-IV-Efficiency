/*
* This file is used for code metric subroutines to help calculate various metrics of source code
* v1.0 
*	- Timer : Use start to begin timing a function and getTime to get time elapsed in ms since the start function was called
*	- Calculation : used to get averages or other caluclations based on passed in data values
*/
#pragma once
#include <Windows.h>
#include <chrono>
#include <vector>
#include "Logger.h"
#include <filesystem>

using namespace std;
using namespace std::chrono;

#define METRICS // used to set metrics run or not

/// <summary>
/// Namespace is used for ease of use and quality of life calculations of code metrics
/// </summary>
namespace Metrics {
	
	const string clientMetricsLogFileName = "Client - Metrics";
	const string serverMetricsLogFileName = "Server - Metrics";
	Logger logger;

	/// <summary>
	/// used to simplify the calculation of code timings using the std::chrono library
	/// </summary>
	class Timer {
		steady_clock::time_point startTime;
	public:
		/// <summary>
		/// starts a timer
		/// </summary>
		bool start() {
			try {
				startTime = high_resolution_clock::now();
				return true;
			}
			catch (exception e) {
				return false;
			}
		}
		/// <summary>
		/// returns the time since the timer was started in milliseconds
		/// </summary>
		/// <returns></returns>
		float getTime() {
			steady_clock::time_point endTime = high_resolution_clock::now();

			return float((endTime - startTime).count())/1000000;

		}
	};

	/// <summary>
	/// helps to calculate averages of metrics quickly and easily
	/// </summary>
	class Calculations {
		float sum = 0;
		int count = 0;
	public:

		/// <summary>
		/// add a datapoint such as time at a given index t(n) add times such as 0.00734ms 
		/// </summary>
		/// <param name="point"></param>
		void addPoint(float point) {
			sum += point;
			count++;
		}

		/// <summary>
		/// get average for the points added such as times are 1s 3s, 7s therefor total averafe is 11/3
		/// </summary>
		/// <returns></returns>
		float getAverage() {
			return sum / count;
		}

		/// <summary>
		/// gets the sum for all the points added to this calculations object
		/// </summary>
		/// <returns></returns>
		float getSum() {
			return sum;
		}
	};
	/// <summary>
	/// prints system statistics using wmic
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void logSystemStatsMetrics(bool clientOrServer) {
		// log and archive system information
		string logFilePath;
		string archiveFilePath;

		if ( clientOrServer ) {
			logger.log("Client - Metrics", clientMetricsLogFileName);
			logger.log("-------------------------------------------------------------------------------", clientMetricsLogFileName);
			logFilePath = "\"%cd%/../Logs/" + logger.getFileTimeName() + clientMetricsLogFileName + ".log\"";
			archiveFilePath = "\"%cd%/../Archive/" + clientMetricsLogFileName + ".archive\"";
		} else {
			logger.log("Server - Metrics", serverMetricsLogFileName);
			logger.log("-------------------------------------------------------------------------------", serverMetricsLogFileName);
			logFilePath = "\"%cd%/../Logs/" + logger.getFileTimeName() + serverMetricsLogFileName + ".log\"";
			archiveFilePath = "\"%cd%/../Archive/" + serverMetricsLogFileName + ".archive\"";
		}

		// log and archive system information
		system((std::string("wmic cpu get CurrentClockSpeed, MaxClockSpeed, Name, CurrentVoltage, DataWidth, ProcessorType | findstr /r /v \"^$\" | more >> " + logFilePath)).c_str());
		system((std::string("wmic cpu get CurrentClockSpeed, MaxClockSpeed, Name, CurrentVoltage, DataWidth, ProcessorType | findstr /r /v \"^$\" | more >> " + archiveFilePath)).c_str());

		system((std::string("wmic memorychip get FormFactor, Speed, Capacity, DataWidth, Manufacturer, name | findstr /r /v \"^$\" | more >> " + logFilePath)).c_str());
		system((std::string("wmic memorychip get FormFactor, Speed, Capacity, DataWidth, Manufacturer, name | findstr /r /v \"^$\" | more >> " + archiveFilePath)).c_str());

		system((std::string("wmic diskdrive get manufacturer, size,name, model, description | findstr /r /v \"^$\" | more >> " + logFilePath)).c_str());
		system((std::string("wmic diskdrive get manufacturer, size,name, model, description | findstr /r /v \"^$\" | more >> " + archiveFilePath)).c_str());

	}


	/// <summary>
	/// Takes in a counter for times, a line counter, and a measure of timein milliseconds for getTime to run, specific to current implementation 01/26/2023
	/// </summary>
	/// <param name="calculations"></param>
	/// <param name="lineCounter"></param>
	/// <param name=""></param>
	/// <param name=""></param>
	void logClientIOMetrics(Metrics::Calculations calculations, Metrics::Calculations lineCounter, float timeToGetSize) {
		
		Timer timer;

		// log information from counters and timers
		logger.log("Client - IO - Get File Size :" + to_string(timeToGetSize) + "ms", clientMetricsLogFileName);
		logger.log("Client - IO - Average time to get line from file : " + to_string(calculations.getAverage()) + "ms", clientMetricsLogFileName);
		logger.log("Client - IO - TotalTime reading files to get specific lines : " + to_string(calculations.getSum()) + "ms", clientMetricsLogFileName);
		logger.log("Client - IO - Total lines reading files ( not including get file length ) : " + to_string(int(lineCounter.getSum())), clientMetricsLogFileName);

		// get file counts, plus total bytes of data from all .txt files
		int fileCounter = 0; // to count number of txt files
		int byteCounter = 0; // to get total number of bytes
		for (const auto& entry : std::filesystem::directory_iterator("../Client/")) {
			if (entry.path().extension().string() == ".txt") {
				byteCounter+=std::filesystem::file_size(entry.path());
				fileCounter++;
			}
		}
		// log data about file counts and bytes
		logger.log("Client - IO - Data File Count (.txt) is : " + to_string(fileCounter), clientMetricsLogFileName);
		logger.log("Client - IO - Total bytes in data files is : " + to_string(byteCounter), clientMetricsLogFileName);
		logger.emptyLine(clientMetricsLogFileName);
	}

	void logDataParsingMetricsClient(Calculations dataParsingTimeCalc, Calculations sizeOfDataParsedDataClientCalc, int numDataParsesClient){
		//data parsing results
		logger.log("Client - DataParsing - Total Time = " + to_string(dataParsingTimeCalc.getSum()) + " ms", clientMetricsLogFileName);
		logger.log("Client - DataParsing - Average (Single Parse) = " + to_string(dataParsingTimeCalc.getAverage()) + " ms", clientMetricsLogFileName);
		logger.log("Client - DataParsing - # of Conversions = " + to_string(numDataParsesClient), clientMetricsLogFileName);
		logger.log("Client - DataParsing - Input Size of Parsed Data = " + to_string(std::filesystem::file_size("DataFile.txt")) + " Bytes", clientMetricsLogFileName);
		logger.log("Client - DataParsing - Total Size of Parsed Data = " + to_string((int)sizeOfDataParsedDataClientCalc.getSum()) + " Bytes", clientMetricsLogFileName);
		logger.emptyLine(clientMetricsLogFileName);
	}

	void logDataParsingMetricsServer(Calculations dataParsingTimeCalc, Calculations sizeOfDataParsedDataServerCalc, int numDataParsesServer){
		//data parsing results
		logger.log("Server - DataParsing - Total Time = " + to_string(dataParsingTimeCalc.getSum()) + " ms", serverMetricsLogFileName);
		logger.log("Server - DataParsing - Average (Single Parse) = " + to_string(dataParsingTimeCalc.getAverage()) + " ms", serverMetricsLogFileName);
		logger.log("Server - DataParsing - # of Conversions = " + to_string(numDataParsesServer), serverMetricsLogFileName);
		logger.log("Server - DataParsing - Total Size of Parsed Data = " + to_string((int)sizeOfDataParsedDataServerCalc.getSum()) + " Bytes", serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}

	void logCalcInfo(float calcTime, int numCalc) {
		//calculation results
		logger.log("Server - Calculations - Average time used for a calculation: " + to_string((calcTime/numCalc)*1000) + " µs", serverMetricsLogFileName);
		logger.log("Server - Calculations - Total time used for calculation: " + to_string(calcTime) + " ms", serverMetricsLogFileName);
		logger.log("Server - Calculations - Total number of calculations done: " + to_string(numCalc) + " ms", serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}

	void logMemoryMetricsServer(Calculations sizeOfMemoryServerCalc){
		//memory results
		logger.log("Server - Memory - Total Memory Allocated: " + to_string(sizeOfMemoryServerCalc.getSum()) + " Bytes", serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}

	void logNetworkMetricsClient(int numTransmissions, int avgHandshake, int handshakeTransmissionCount, string networkType) {
		// Client Network Results
		logger.log("Client - Network - Network Type: " + networkType, clientMetricsLogFileName);
		logger.log("Client - Network - Number of Transmissions: " + to_string(numTransmissions), clientMetricsLogFileName);
		logger.log("Client - Network - Average Handshake Time: " + to_string(avgHandshake) +  " µs", clientMetricsLogFileName);
		logger.log("Client - Network - Number of Transmissions in Handshake: " + to_string(handshakeTransmissionCount), clientMetricsLogFileName);
		logger.emptyLine(clientMetricsLogFileName);
		
	}

	void logNetworkMetricsServer(int elapsedTimeMilSec, int numConnections) {
		// Server Network Results
		logger.log("Server - Network - Server Uptime: " + to_string(elapsedTimeMilSec) + " ms", serverMetricsLogFileName);
		logger.log("Server - Network - Number of Connections: " + to_string(numConnections), serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}
	/// <summary>
	/// calls logger function on appropriate target
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void addLogEndOfFileSpacing(bool clientOrServer){
		for(int i = 0; i < 3; i++){
			if ( clientOrServer ) {
				logger.addLogEndOfFileSpacingArchive(clientMetricsLogFileName);
			} else {
				logger.addLogEndOfFileSpacingArchive(serverMetricsLogFileName);
			}
		}
	}

}