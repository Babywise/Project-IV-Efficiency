#pragma once


#include <Windows.h>
#include <chrono>
#include <vector>
#include "Logger.h"
#include <filesystem>


using namespace std::chrono;

//#define METRICS // used to set metrics run or not

/// <summary>
/// Namespace is used for ease of use and quality of life calculations of code metrics
/// </summary>
namespace Metrics {

	std::string clientMetricsLogFileName;
	std::string serverMetricsLogFileName;
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
			} catch (std::exception e) {
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

	void setClientLogName(std::string clientMetricsLogFileName) {
		Metrics::clientMetricsLogFileName = clientMetricsLogFileName;
	}

	void setServerLogName(std::string serverMetricsLogFileName) {
		Metrics::serverMetricsLogFileName = serverMetricsLogFileName;
	}

	/// <summary>
	/// calls logger function on appropriate target
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void addLogDivider(bool clientOrServer) {
		if (clientOrServer) {
			logger.log("-----------------------------------------------------------------------------------", clientMetricsLogFileName, true);
			logger.emptyLine(clientMetricsLogFileName, true);
		} else {
			logger.log("-----------------------------------------------------------------------------------", serverMetricsLogFileName, false);
			logger.emptyLine(serverMetricsLogFileName, false);
		}
	}

	void logStartOfServer() {

		logger.log("Server - Metrics", serverMetricsLogFileName, false);
		addLogDivider(false);

	}

	void logStartOfClient(char* filename, int planeID) {
		std::string fileNameStr = filename;
		logger.log("Client - Metrics | ID: " + std::to_string(planeID) + " | " + fileNameStr, clientMetricsLogFileName, true);
		addLogDivider(true);

	}

	/// <summary>
	/// prints system statistics using wmic
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void logSystemStatsMetrics(bool clientOrServer) {
		// log and archive system information
		std::string logFilePath;
		std::string archiveFilePath;

		if (clientOrServer) {
			logFilePath = "\"%cd%/../Logs/Client/" + logger.getFileTimeName() + clientMetricsLogFileName + ".log\"";
			archiveFilePath = "\"%cd%/../Archive/" + std::string("Client - Metrics") + ".archive\"";
		} else {
			logFilePath = "\"%cd%/../Logs/Server/" + logger.getFileTimeName() + serverMetricsLogFileName + ".log\"";
			archiveFilePath = "\"%cd%/../Archive/" + std::string("Server - Metrics") + ".archive\"";
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
		logger.log("Client - IO - Get File Size : " + std::to_string(timeToGetSize) + " ms", clientMetricsLogFileName, true);
		logger.log("Client - IO - Average time to get line from file : " + std::to_string(calculations.getAverage()) + " ms", clientMetricsLogFileName, true);
		logger.log("Client - IO - TotalTime reading files to get specific lines : " + std::to_string(calculations.getSum()) + " ms", clientMetricsLogFileName, true);
		logger.log("Client - IO - Total lines reading files ( not including get file length ) : " + std::to_string(int(lineCounter.getSum())), clientMetricsLogFileName, true);

		// get file counts, plus total bytes of data from all .txt files
		int fileCounter = 0; // to count number of txt files
		int byteCounter = 0; // to get total number of bytes
		for (const auto& entry : std::filesystem::directory_iterator("./")) {
			if (entry.path().extension().string() == ".txt") {
				byteCounter += std::filesystem::file_size(entry.path());
				fileCounter++;
			}
		}
		// log data about file counts and bytes
		logger.log("Client - IO - Data File Count (.txt) is : " + std::to_string(fileCounter), clientMetricsLogFileName, true);
		logger.log("Client - IO - Total bytes in data files is : " + std::to_string(byteCounter), clientMetricsLogFileName, true);
		logger.emptyLine(clientMetricsLogFileName, true);
	}

	void logDataParsingMetricsClient(Calculations dataParsingTimeCalc, Calculations sizeOfDataParsedDataClientCalc, int numDataParsesClient) {
		//data parsing results
		logger.log("Client - DataParsing - Total Time = " + std::to_string(dataParsingTimeCalc.getSum()) + " ms", clientMetricsLogFileName, true);
		logger.log("Client - DataParsing - Average (Single Parse) = " + std::to_string(dataParsingTimeCalc.getAverage()) + " ms", clientMetricsLogFileName, true);
		logger.log("Client - DataParsing - # of Conversions = " + std::to_string(numDataParsesClient), clientMetricsLogFileName, true);
		logger.log("Client - DataParsing - Total Size of Parsed Data = " + std::to_string((int)sizeOfDataParsedDataClientCalc.getSum() / 1000) + " KB", clientMetricsLogFileName, true);
		logger.emptyLine(clientMetricsLogFileName, true);
	}

	void logDataParsingMetricsServer(Calculations dataParsingTimeCalc, Calculations sizeOfDataParsedDataServerCalc, int numDataParsesServer) {
		//data parsing results
		logger.log("Server - DataParsing - Total Time = " + std::to_string(dataParsingTimeCalc.getSum()) + " ms", serverMetricsLogFileName, false);
		logger.log("Server - DataParsing - Average (Single Parse) = " + std::to_string(dataParsingTimeCalc.getAverage()) + " ms", serverMetricsLogFileName, false);
		logger.log("Server - DataParsing - # of Conversions = " + std::to_string(numDataParsesServer), serverMetricsLogFileName, false);
		logger.log("Server - DataParsing - Total Size of Parsed Data = " + std::to_string((int)sizeOfDataParsedDataServerCalc.getSum() / 1000) + " KB", serverMetricsLogFileName, false);
		logger.emptyLine(serverMetricsLogFileName, false);
		addLogDivider(false);
	}

	void logCalcInfo(float calcTime, int numCalc) {
		//calculation results
		logger.log("Server - Calculations - Average time used for a calculation: " + std::to_string((calcTime / numCalc) * 1000) + " µs", serverMetricsLogFileName, false);
		logger.log("Server - Calculations - Total time used for calculation: " + std::to_string(calcTime) + " ms", serverMetricsLogFileName, false);
		logger.log("Server - Calculations - Total number of calculations done: " + std::to_string(numCalc) + " ms", serverMetricsLogFileName, false);
		logger.emptyLine(serverMetricsLogFileName, false);
	}

	void logMemoryMetricsServer(Calculations sizeOfMemoryServerCalc) {
		//memory results
		logger.log("Server - Memory - Total Memory Allocated: " + std::to_string(sizeOfMemoryServerCalc.getSum()) + " Bytes", serverMetricsLogFileName, false);
		logger.emptyLine(serverMetricsLogFileName, false);
	}

	void logNetworkMetricsClient(int numTransmissions, int avgHandshake, int handshakeTransmissionCount, std::string networkType) {
		// Client Network Results
		logger.log("Client - Network - Network Type: " + networkType, clientMetricsLogFileName, true);
		logger.log("Client - Network - Number of Transmissions: " + std::to_string(numTransmissions), clientMetricsLogFileName, true);
		logger.log("Client - Network - Average Handshake Time: " + std::to_string(avgHandshake) + " µs", clientMetricsLogFileName, true);
		logger.log("Client - Network - Number of Transmissions in Handshake: " + std::to_string(handshakeTransmissionCount), clientMetricsLogFileName, true);
		logger.emptyLine(clientMetricsLogFileName, true);
	}

	void logNetworkMetricsServer(int planeID, auto currentUptime, int numTotalConnections,
		int numCurrentConnections, int numCompletedConnections, int numFailedConnections, std::string errMessage) {

		// Server Network Results
		logger.log("Server - Network - PlaneID: " + std::to_string(planeID), serverMetricsLogFileName, false);

		if (currentUptime.count() < 60000) { // less than 60 seconds
			logger.log("Server - Network - Current Server Uptime: " + std::to_string(currentUptime.count()) + " ms", serverMetricsLogFileName, false);
		} else if (currentUptime.count() >= 60000 && currentUptime.count() <= 600000) { // between 60 seconds (1 minute) and 600 seconds (10 minutes)
			logger.log("Server - Network - Current Server Uptime: " + std::to_string(currentUptime.count() / 1000) + " s", serverMetricsLogFileName, false);
		} else { // Anything larger than 10 minutes
			logger.log("Server - Network - Current Server Uptime: " + std::to_string((currentUptime.count() / 1000) / 60) + " mins", serverMetricsLogFileName, false);
		}

		logger.log("Server - Network - Total Connection Counter: " + std::to_string(numTotalConnections), serverMetricsLogFileName, false);
		logger.log("Server - Network - Current Connection Counter: " + std::to_string(numCurrentConnections), serverMetricsLogFileName, false);
		logger.log("Server - Network - Completed Connection Counter: " + std::to_string(numCompletedConnections), serverMetricsLogFileName, false);
		logger.log("Server - Network - Failed Connection Counter: " + std::to_string(numFailedConnections), serverMetricsLogFileName, false);
		if (errMessage != "") { logger.log("Server - Network - Reason For Failure: " + errMessage, serverMetricsLogFileName, false); }
		logger.emptyLine(serverMetricsLogFileName, false);
	}

	void logFlightStatisticsServer(Packet plane, float startingFuel) {
		logger.log("Server - Flight Statistics - PlaneID: " + std::to_string(plane.getPlaneID()), serverMetricsLogFileName, false);

		logger.log("Flight ID: " + std::to_string(plane.getPlaneID()), serverMetricsLogFileName, false);
		logger.log("Flight Duration (seconds): " + plane.getTimestamp(), serverMetricsLogFileName, false);
		logger.log("Flight Starting Fuel: " + std::to_string(startingFuel), serverMetricsLogFileName, false);
		logger.log("Flight Ending Fuel: " + std::to_string(startingFuel - plane.getCurrentFuelConsumption()), serverMetricsLogFileName, false);
		logger.log("Flight Total Fuel Consumption: " + std::to_string(plane.getCurrentFuelConsumption()), serverMetricsLogFileName, false);
		logger.log("Flight Average Fuel: " + std::to_string(plane.getAverageFuelConsumption()), serverMetricsLogFileName, false);

		logger.emptyLine(serverMetricsLogFileName, false);
	}


	/// <summary>
	/// calls logger function on appropriate target
	/// </summary>
	/// <param name="clientOrServer">Client = True, Server = False</param>
	void addLogEndOfFileSpacing(bool clientOrServer) {
		if (clientOrServer) {
			logger.addLogEndOfFileSpacingArchive("Client - Metrics", true);
		} else {
			logger.addLogEndOfFileSpacingArchive("Server - Metrics", false);
		}
	}

}