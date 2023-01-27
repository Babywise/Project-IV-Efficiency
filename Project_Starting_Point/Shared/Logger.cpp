#define _CRT_SECURE_NO_WARNINGS

// includes
#include "Logger.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <filesystem>

//define statements
#define LOGSPATH "../Logs/"
#define DEFAULT_LOG "log"
#define LOGEXTENSION ".log"
#define ARCHIVE "../Archive/"
#define ARCHIVE_EXT ".archive"

/// <summary>
/// Log to the default log path ./Logs/Log.log using the input string
/// </summary>
/// <param name="message"></param>
void Logger::log(std::string message)
{
	Logger::log(message, -1, DEFAULT_LOG);

}

/// <summary>
/// Logs to the default log ./Logs/Log.log using the input string and severity
/// </summary>
/// <param name="message"></param>
/// <param name="severity"></param>
void Logger::log(std::string message, int severity)
{
	Logger::log(message, severity, DEFAULT_LOG);

}

/// <summary>
/// Logs to the given log name in the default route ./Logs/%logname%.log using the message and severity provided.
/// </summary>
/// <param name="message"></param>
/// <param name="severity"></param>
/// <param name="logname"></param>
void Logger::log(std::string message, int severity, std::string logname)
{

	
	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	outfile.open((std::string)LOGSPATH + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);

	outfile << message << "\t\t Severity :" << severity << "\t\t" << std::ctime(&end_time);
	outfile.close();
}
/// <summary>
/// this log function is primarily designed for storing metrics without severity
/// </summary>
/// <param name="message"></param>
/// <param name="logname"></param>
void Logger::log(std::string message, std::string logname)
{



	if (!std::filesystem::exists(LOGSPATH)) {
		std::filesystem::create_directory(LOGSPATH);
	}
	Logger archive;
	archive.Archive(message, logname);
	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	outfile.open((std::string)LOGSPATH + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);

	outfile << message << "\t\t" << std::ctime(&end_time);
	outfile.close();
}

/// <summary>
/// archive function used in conjunction with the log function to log to a hidden folder for secure long term storage
/// </summary>
/// <param name="message"></param>
/// <param name="logname"></param>
void Logger::Archive(std::string message, std::string logname)
{

	if (!std::filesystem::exists(ARCHIVE)) {
		std::filesystem::create_directory(ARCHIVE);
		system("attrib +h \"%cd%/../Archive\"");
		
	}
	
	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	outfile.open((std::string)ARCHIVE + (std::string)logname + (std::string)ARCHIVE_EXT, std::ios::app);

	outfile << message << "\t\t" << std::ctime(&end_time);
	outfile.close();

}

void Logger::emptyLine(std::string logname) {
	std::ofstream outfile;
	outfile.open((std::string)LOGSPATH + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	Logger c;
	c.Archive("\n", logname);
	outfile << "\n";
	outfile.close();
}
