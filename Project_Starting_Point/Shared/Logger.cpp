#define _CRT_SECURE_NO_WARNINGS
#include "Logger.h"
/// <summary>
/// Assigns Global variable fileTimeName - Use ONCE in program before calling any other log function
/// </summary>
void Logger::generateFileTime()
{
	std::time_t fileTimeCreation = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char* ctimeBuf = std::ctime(&fileTimeCreation);
	if ( ctimeBuf[strlen(ctimeBuf) - 1] == '\n' ) ctimeBuf[strlen(ctimeBuf) - 1] = '\0';
	for ( int i = 0; i < strlen(ctimeBuf) - 1; i++) {
		if ( ctimeBuf[i] == ':' ) {
			ctimeBuf[i] = '-';
		}
	}
	this->fileTimeName = ctimeBuf;
	this->fileTimeName += " - ";
}
/// <summary>
/// Logger Constructor - calls generateFileTime
/// </summary>
Logger::Logger()
{
	Logger::generateFileTime();
}

std::string Logger::getFileTimeName()
{
	return this->fileTimeName;
}

/// <summary>
/// Log to the default log path ./Logs/Log.log using the input string
/// </summary>
/// <param name="message"></param>
/// <param name="clientOrServer">Client = True, Server = False</param>
void Logger::log(std::string message, bool clientOrServer)
{
	Logger::log(message, -1, DEFAULT_LOG, clientOrServer);

}

/// <summary>
/// Logs to the default log ./Logs/Log.log using the input string and severity
/// </summary>
/// <param name="message"></param>
/// <param name="severity"></param>
/// <param name="clientOrServer">Client = True, Server = False</param>
void Logger::log(std::string message, int severity, bool clientOrServer)
{
	Logger::log(message, severity, DEFAULT_LOG, clientOrServer);

}

/// <summary>
/// Logs to the given log name in the default route ./Logs/%logname%.log using the message and severity provided.
/// </summary>
/// <param name="message"></param>
/// <param name="severity"></param>
/// <param name="logname"></param>
/// <param name="clientOrServer">Client = True, Server = False</param>
void Logger::log(std::string message, int severity, std::string logname, bool clientOrServer)
{
	if ( !std::filesystem::exists(LOGSPATH) ) {
		std::filesystem::create_directory(LOGSPATH);
	}

	Logger archive;

	if ( clientOrServer ) {
		if ( !std::filesystem::exists(LOGSPATHCLIENT) ) {
			std::filesystem::create_directory(LOGSPATHCLIENT);
		}
		archive.Archive(message, logname, true);
	} else {
		if ( !std::filesystem::exists(LOGSPATHSERVER) ) {
			std::filesystem::create_directory(LOGSPATHSERVER);
		}
		archive.Archive(message, logname, false);
	}

	
	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	if ( clientOrServer ) {
		outfile.open((std::string)LOGSPATHCLIENT + (std::string)this->fileTimeName + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	} else {
		outfile.open((std::string)LOGSPATHSERVER + (std::string)this->fileTimeName + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	}
	char* ctimeBuf = std::ctime(&end_time);
	if ( ctimeBuf[strlen(ctimeBuf) - 1] == '\n' ) ctimeBuf[strlen(ctimeBuf) - 1] = '\0';
	// replace endl with \0
	outfile << ctimeBuf << "\t Severity :" << severity << "\t" << message << std::endl;
	outfile.close();
}
/// <summary>
/// this log function is primarily designed for storing metrics without severity
/// </summary>
/// <param name="message"></param>
/// <param name="logname"></param>
/// <param name="clientOrServer">Client = True, Server = False</param>
void Logger::log(std::string message, std::string logname, bool clientOrServer)
{
	if ( !std::filesystem::exists(LOGSPATH) ) {
		std::filesystem::create_directory(LOGSPATH);
	}

	Logger archive;

	if ( clientOrServer ) {
		if ( !std::filesystem::exists(LOGSPATHCLIENT) ) {
			std::filesystem::create_directory(LOGSPATHCLIENT);
		}
		archive.Archive(message, logname, true);
	} else {
		if ( !std::filesystem::exists(LOGSPATHSERVER) ) {
			std::filesystem::create_directory(LOGSPATHSERVER);
		}
		archive.Archive(message, logname, false);
	}

	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	if ( clientOrServer ) {
		outfile.open((std::string)LOGSPATHCLIENT + (std::string)this->fileTimeName + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	} else {
		outfile.open((std::string)LOGSPATHSERVER + (std::string)this->fileTimeName + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	}
	char* ctimeBuf = std::ctime(&end_time);
	// replace endl with \0
	if ( ctimeBuf[strlen(ctimeBuf) - 1] == '\n' ) ctimeBuf[strlen(ctimeBuf) - 1] = '\0';
	outfile << ctimeBuf << "\t" << message << std::endl;
	outfile.close();
}

/// <summary>
/// archive function used in conjunction with the log function to log to a hidden folder for secure long term storage
/// </summary>
/// <param name="message"></param>
/// <param name="logname"></param>
void Logger::Archive(std::string message, std::string logname, bool clientOrServer)
{

	if (!std::filesystem::exists(ARCHIVE)) {
		std::filesystem::create_directory(ARCHIVE);
		system("attrib +h \"%cd%/../Archive\"");
		
	}
	
	auto now = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream outfile;
	if ( clientOrServer ) {
		outfile.open((std::string)ARCHIVE + "Client - Metrics" + (std::string)ARCHIVE_EXT, std::ios::app);
	} else {
		outfile.open((std::string)ARCHIVE + "Server - Metrics" + (std::string)ARCHIVE_EXT, std::ios::app);
	}

	char* ctimeBuf = std::ctime(&end_time);
	// replace endl with \0
	if ( ctimeBuf[strlen(ctimeBuf) - 1] == '\n' ) ctimeBuf[strlen(ctimeBuf) - 1] = '\0';
	outfile << ctimeBuf << "\t" << message << std::endl;
	outfile.close();
	if ( clientOrServer ) {
		system(std::string("attrib +h \"%cd%/../Archive/" + std::string("Client - Metrics") + ARCHIVE_EXT + "\"").c_str());
	} else {
		system(std::string("attrib +h \"%cd%/../Archive/" + std::string("Server - Metrics") + ARCHIVE_EXT + "\"").c_str());
	}

}
/// <summary>
/// adds a new line in both the archive and log files
/// </summary>
/// <param name="logname"></param>
/// <param name="clientOrServer">Client = True, Server = False</param>
void Logger::emptyLine(std::string logname, bool clientOrServer) {
	std::ofstream outfile;
	if ( clientOrServer ) {
		outfile.open((std::string)LOGSPATHCLIENT + (std::string)this->fileTimeName + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	} else {
		outfile.open((std::string)LOGSPATHSERVER + (std::string)this->fileTimeName + (std::string)logname + (std::string)LOGEXTENSION, std::ios::app);
	}
	outfile << std::endl;
	outfile.close();
	
	if ( clientOrServer ) {
		outfile.open((std::string)ARCHIVE + std::string("Client - Metrics") + (std::string)ARCHIVE_EXT, std::ios::app);
	} else {
		outfile.open((std::string)ARCHIVE + std::string("Server - Metrics") + (std::string)ARCHIVE_EXT, std::ios::app);
	}
	outfile << std::endl;
	outfile.close();
}
/// <summary>
/// adds 3 new lines (padding) to the archive file
/// </summary>
/// <param name="logname"></param>
/// <param name="clientOrServer">Client = True, Server = False</param>
void Logger::addLogEndOfFileSpacingArchive(std::string logname, bool clientOrServer)
{
	std::ofstream outfile;
	if ( clientOrServer ) {
		outfile.open((std::string)ARCHIVE + std::string("Client - Metrics") + (std::string)ARCHIVE_EXT, std::ios::app);
	} else {
		outfile.open((std::string)ARCHIVE + std::string("Server - Metrics") + (std::string)ARCHIVE_EXT, std::ios::app);
	}
	outfile << std::endl;
	outfile << std::endl;
	outfile << std::endl;
	outfile.close();
}
