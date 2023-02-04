#pragma once
#include <string>
class Logger {
public:
	void static log(std::string message);
	void static log(std::string message, int severity);
	void static log(std::string message,int severity,std::string logname);
	void static log(std::string message, std::string logName);
	void Archive(std::string message, std::string logname);
	void static emptyLine(std::string logname);
};