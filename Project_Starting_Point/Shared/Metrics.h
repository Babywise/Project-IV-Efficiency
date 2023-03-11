#pragma once


#include <Windows.h>
#include <chrono>
#include <vector>
#include "Logger.h"
#include <filesystem>


using namespace std::chrono;

#define METRICS // used to set metrics run or not

/// <summary>
/// Namespace is used for ease of use and quality of life calculations of code metrics
/// </summary>
namespace Metrics {
	
	const std::string clientMetricsLogFileName = "Client - Metrics";
	const std::string serverMetricsLogFileName = "Server - Metrics";
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
			catch (std::exception e) {
				return false;
			}
		}
		/// <summary>
		/// returns the time since the timer was started in milliseconds
		/// </summary>
		/// <returns></returns>
		float getTime() {
			steady_clock::time_point endTime = high_resolution_clock::now();

			return float((endTime - startTime).count()) / 1000000;

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

	void logStartOfServer() {
		
		logger.log("Server - Metrics", serverMetricsLogFileName);
		logger.log("-----------------------------------------------------------------------------------", serverMetricsLogFileName);
		
	}

	void logStartOfClient(char* filename, int planeID) {
		std::string fileNameStr = filename;
		logger.log("Client - Metrics | ID: " + std::to_string(planeID) + " | " + fileNameStr, clientMetricsLogFileName);
		logger.log("-----------------------------------------------------------------------------------", clientMetricsLogFileName);

	}

	/// <summary>
	/// prints system statistics using wmic
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void logSystemStatsMetrics(bool clientOrServer) {
		// log and archive system information
		std::string logFilePath;
		std::string archiveFilePath;

		if ( clientOrServer ) {
			logFilePath = "\"%cd%/../Logs/" + logger.getFileTimeName() + clientMetricsLogFileName + ".log\"";
			archiveFilePath = "\"%cd%/../Archive/" + clientMetricsLogFileName + ".archive\"";
		} else {
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
		logger.log("Client - IO - Get File Size :" + std::to_string(timeToGetSize) + "ms", clientMetricsLogFileName);
		logger.log("Client - IO - Average time to get line from file : " + std::to_string(calculations.getAverage()) + "ms", clientMetricsLogFileName);
		logger.log("Client - IO - TotalTime reading files to get specific lines : " + std::to_string(calculations.getSum()) + "ms", clientMetricsLogFileName);
		logger.log("Client - IO - Total lines reading files ( not including get file length ) : " + std::to_string(int(lineCounter.getSum())), clientMetricsLogFileName);

		// get file counts, plus total bytes of data from all .txt files
		int fileCounter = 0; // to count number of txt files
		int byteCounter = 0; // to get total number of bytes
		for (const auto& entry : std::filesystem::directory_iterator("../Client/")) {
			if (entry.path().extension().string() == ".txt") {
				byteCounter += std::filesystem::file_size(entry.path());
				fileCounter++;
			}
		}
		// log data about file counts and bytes
		logger.log("Client - IO - Data File Count (.txt) is : " + std::to_string(fileCounter), clientMetricsLogFileName);
		logger.log("Client - IO - Total bytes in data files is : " + std::to_string(byteCounter), clientMetricsLogFileName);
		logger.emptyLine(clientMetricsLogFileName);
	}

	void logDataParsingMetricsClient(Calculations dataParsingTimeCalc, Calculations sizeOfDataParsedDataClientCalc, int numDataParsesClient) {
		//data parsing results
		logger.log("Client - DataParsing - Total Time = " + std::to_string(dataParsingTimeCalc.getSum()) + " ms", clientMetricsLogFileName);
		logger.log("Client - DataParsing - Average (Single Parse) = " + std::to_string(dataParsingTimeCalc.getAverage()) + " ms", clientMetricsLogFileName);
		logger.log("Client - DataParsing - # of Conversions = " + std::to_string(numDataParsesClient), clientMetricsLogFileName);
		logger.log("Client - DataParsing - Input Size of Parsed Data = " + std::to_string(std::filesystem::file_size("DataFile.txt")) + " Bytes", clientMetricsLogFileName);
		logger.log("Client - DataParsing - Total Size of Parsed Data = " + std::to_string((int)sizeOfDataParsedDataClientCalc.getSum()) + " Bytes", clientMetricsLogFileName);
		logger.emptyLine(clientMetricsLogFileName);
	}

	void logDataParsingMetricsServer(Calculations dataParsingTimeCalc, Calculations sizeOfDataParsedDataServerCalc, int numDataParsesServer) {
		//data parsing results
		logger.log("Server - DataParsing - Total Time = " + std::to_string(dataParsingTimeCalc.getSum()) + " ms", serverMetricsLogFileName);
		logger.log("Server - DataParsing - Average (Single Parse) = " + std::to_string(dataParsingTimeCalc.getAverage()) + " ms", serverMetricsLogFileName);
		logger.log("Server - DataParsing - # of Conversions = " + std::to_string(numDataParsesServer), serverMetricsLogFileName);
		logger.log("Server - DataParsing - Total Size of Parsed Data = " + std::to_string((int)sizeOfDataParsedDataServerCalc.getSum()) + " Bytes", serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}

	void logCalcInfo(float calcTime, int numCalc) {
		//calculation results
		logger.log("Server - Calculations - Average time used for a calculation: " + std::to_string((calcTime/numCalc)*1000) + " µs", serverMetricsLogFileName);
		logger.log("Server - Calculations - Total time used for calculation: " + std::to_string(calcTime) + " ms", serverMetricsLogFileName);
		logger.log("Server - Calculations - Total number of calculations done: " + std::to_string(numCalc) + " ms", serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}

	void logMemoryMetricsServer(Calculations sizeOfMemoryServerCalc) {
		//memory results
		logger.log("Server - Memory - Total Memory Allocated: " + std::to_string(sizeOfMemoryServerCalc.getSum()) + " Bytes", serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}

	void logNetworkMetricsClient(int numTransmissions, int avgHandshake, int handshakeTransmissionCount, std::string networkType) {
		// Client Network Results
		logger.log("Client - Network - Network Type: " + networkType, clientMetricsLogFileName);
		logger.log("Client - Network - Number of Transmissions: " + std::to_string(numTransmissions), clientMetricsLogFileName);
		logger.log("Client - Network - Average Handshake Time: " + std::to_string(avgHandshake) +  " µs", clientMetricsLogFileName);
		logger.log("Client - Network - Number of Transmissions in Handshake: " + std::to_string(handshakeTransmissionCount), clientMetricsLogFileName);
		logger.emptyLine(clientMetricsLogFileName);
		
	}

	void logNetworkMetricsServer(int planeID, auto currentUptime, int numConnections) {
		// Server Network Results
		logger.log("Server - Network - PlaneID: " + std::to_string(planeID), serverMetricsLogFileName);
		logger.log("Server - Network - Current Server Uptime: " + std::to_string(currentUptime.count()) + "ms", serverMetricsLogFileName);
		logger.log("Server - Network - Connection Counter: " + std::to_string(numConnections), serverMetricsLogFileName);
		logger.emptyLine(serverMetricsLogFileName);
	}
	/// <summary>
	/// calls logger function on appropriate target
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void addLogEndOfFileSpacing(bool clientOrServer){
		if ( clientOrServer ) {
			logger.addLogEndOfFileSpacingArchive(clientMetricsLogFileName);
		} else {
			logger.addLogEndOfFileSpacingArchive(serverMetricsLogFileName);
		}
	}

}