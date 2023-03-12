#pragma once

// includes
#include <string>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <filesystem>

//define statements
#define LOGSPATH "../Logs/"
#define LOGSPATHCLIENT "../Logs/Client/"
#define LOGSPATHSERVER "../Logs/Server/"
#define DEFAULT_LOG "log"
#define LOGEXTENSION ".log"
#define ARCHIVE "../Archive/"
#define ARCHIVE_EXT ".archive"


class Logger {
	std::string fileTimeName;
	void generateFileTime();
public:
	Logger();
	std::string getFileTimeName();
	void log(std::string message, bool clientOrServer);
	void log(std::string message, int severity, bool clientOrServer);
	void log(std::string message,int severity,std::string logname, bool clientOrServer);
	void log(std::string message, std::string logName, bool clientOrServer);
	void Archive(std::string message, std::string logname, bool clientOrServer);
	void emptyLine(std::string logname, bool clientOrServer);
	void addLogEndOfFileSpacingArchive(std::string logname, bool clientOrServer);
};