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
	void log(std::string message);
	void log(std::string message, int severity);
	void log(std::string message,int severity,std::string logname);
	void log(std::string message, std::string logName);
	void Archive(std::string message, std::string logname);
	void emptyLine(std::string logname);
	void addLogEndOfFileSpacingArchive(std::string logname);
};