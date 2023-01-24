#define _CRT_SECURE_NO_WARNINGS
#include "Logger.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#define LOGSPATH "./Logs/"
#define DEFAULT_LOG "log"
#define LOGEXTENSION ".log"
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


	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	outfile.open((std::string)LOGSPATH + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);

	outfile << message << "\t\t" << std::ctime(&end_time);
	outfile.close();
}
