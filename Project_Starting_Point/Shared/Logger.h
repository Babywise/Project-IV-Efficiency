#pragma once
#include <string>
class Logger {
public:
	void static log(std::string message);
	void static log(std::string message, int severity);
	void static log(std::string message,int severity,std::string logname);
};