/*

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include "../Shared/Packet.h"

#include "../Shared/Logger.h"
#include "../Shared/configManager.h"

configuration::configManager configurations("../Shared/config.conf");
#ifdef METRICS
int numDataParsesServer = 0;
float maxSizeRxData = 0;

Metrics::Timer timer;
Metrics::Calculations dataParsingTimeCalc;
Metrics::Calculations sizeOfDataParsedDataServerCalc;
Metrics::Calculations sizeOfMemoryServerCalc;
#endif



const int numColumns = 7;

struct StorageTypes 
{ 
	unsigned int size = 0;
	float* pData;
};
StorageTypes RxData[numColumns];

void UpdateData(unsigned int, float);
float CalcAvg(unsigned int);
static int numCalc = 0;		//number of calculations
static float calcTime = 0;

/// <summary>
/// main loop that only accepts a single client
/// </summary>
/// <returns></returns>
int main()
{

	//setup
	WSADATA wsaData;
	SOCKET ServerSocket, ConnectionSocket;
	char RxBuffer[1000] = {}; // magic number
#ifdef METRICS
	sizeOfMemoryServerCalc.addPoint(128);
#endif
	sockaddr_in SvrAddr;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == SOCKET_ERROR)
		return -1;

	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = INADDR_ANY;
	SvrAddr.sin_port = htons(atoi(configurations.getConfigChar("port"))); // Magic Number
	bind(ServerSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));

	if (ServerSocket == SOCKET_ERROR)
		return -1;
	// accepts a single connection
	listen(ServerSocket, 1);

#ifdef METRICS

	std::chrono::time_point<std::chrono::system_clock> start, end;

	start = std::chrono::system_clock::now();

#endif

	std::cout << "Waiting for client connection\n" << std::endl;
	ConnectionSocket = SOCKET_ERROR;
	ConnectionSocket = accept(ServerSocket, NULL, NULL);

	if (ConnectionSocket == SOCKET_ERROR)
		return -1;

	int numConnections = 1;

	std::cout << "Connection Established" << std::endl;
	bool exit = false;
	// if first byte of buffer is an asterisk close connection
	while (!exit)
	{
			float fValue = 0;
		memset(RxBuffer, 0, sizeof(RxBuffer));
		recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0); // get param name

		//packet testing
		Packet pRecv(RxBuffer);
		
		send(ConnectionSocket, "ACK", sizeof("ACK"), 0); // send ack
		if (RxBuffer[0] == configurations.getConfigChar("terminator")[0])
			exit = true;
														 
		// find param name recieved
		if (strcmp(RxBuffer, configurations.getConfigChar("columnOne")) == 0)
		{

			

			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0); // get current value for variable
			fValue = (float)atof(RxBuffer);
			UpdateData(0, fValue);// add data to list
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(0);// calculate the average | fValue sends at end of elif
#ifdef METRICS
			calcTime += timer.getTime();
#endif
		}
		else if (strcmp(RxBuffer, configurations.getConfigChar("columnTwo")) == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(1, fValue);
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(1);
#ifdef METRICS
			calcTime += timer.getTime();
#endif		
		}
		else if (strcmp(RxBuffer, configurations.getConfigChar("columnThree")) == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(2, fValue);
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(2);
#ifdef METRICS
			calcTime += timer.getTime();
#endif		
		}
		else if (strcmp(RxBuffer, configurations.getConfigChar("columnFour")) == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(3, fValue);
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(3);
#ifdef METRICS
			calcTime += timer.getTime();
#endif		
		}
		else if (strcmp(RxBuffer, configurations.getConfigChar("columnFive")) == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(4, fValue);
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(4);
#ifdef METRICS
			calcTime += timer.getTime();
#endif		
		}
		else if (strcmp(RxBuffer, configurations.getConfigChar("columnSix")) == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(5, fValue);
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(5);
#ifdef METRICS
			calcTime += timer.getTime();
#endif		
		}
		else if (strcmp(RxBuffer, configurations.getConfigChar("columnSeven")) == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(6, fValue);
#ifdef METRICS
			timer.start();
#endif
			fValue = CalcAvg(6);
#ifdef METRICS
			calcTime += timer.getTime();
#endif
		}
		else // if param not found, reset buffer (sends 0 back to client)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = 0.0;
		}

		char Tx[128];
		// format fvalue to tx and send to client
		sprintf_s(Tx, "%f", fValue);
		send(ConnectionSocket, Tx, sizeof(Tx), 0);//send average back 
	}
#ifdef METRICS
	Metrics::logStartOfServer();
	Metrics::logSystemStatsMetrics(false);
	Metrics::logCalcInfo(calcTime, numCalc);
	Metrics::logDataParsingMetricsServer(dataParsingTimeCalc, sizeOfDataParsedDataServerCalc, numDataParsesServer);
	sizeOfMemoryServerCalc.addPoint(maxSizeRxData);
	Metrics::logMemoryMetricsServer(sizeOfMemoryServerCalc);
#endif
	closesocket(ConnectionSocket);	//closes incoming socket
	closesocket(ServerSocket);	    //closes server socket	

#ifdef METRICS

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsedTimeSeconds = end - start;
	auto elapsedTimeMilSec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTimeSeconds).count();
	Metrics::logNetworkMetricsServer(elapsedTimeMilSec, numConnections);
	Metrics::addLogEndOfFileSpacing(false);
#endif

	WSACleanup();					//frees Winsock resources
	return 1;
}

/// <summary>
/// updates the data in the RxData array with another float value
/// </summary>
/// <param name="uiIndex"></param>
/// <param name="value"></param>
void UpdateData(unsigned int uiIndex, float value)
{
	if (RxData[uiIndex].size == 0) // if first value
	{
		RxData[uiIndex].pData = new float[1]; // init pdata
		RxData[uiIndex].pData[0] = value;
		RxData[uiIndex].size = 1; // size is 1
	}
	else // not first value
	{
		float* pNewData = new float[RxData[uiIndex].size + 1];
		for (unsigned int x = 0; x < RxData[uiIndex].size; x++)
			pNewData[x] = RxData[uiIndex].pData[x]; // set next pdata to data input

		pNewData[RxData[uiIndex].size] = value;
		delete[] RxData[uiIndex].pData; // delete old memory
		RxData[uiIndex].pData = pNewData; // replace with new data added
		RxData[uiIndex].size++;
	}
}

/// <summary>
/// calculates the average for whatever index is passed in ie 1 = body x 2 = body y
/// </summary>
/// <param name="uiIndex"></param>
/// <returns></returns>
float CalcAvg(unsigned int uiIndex)
{
	float Avg = 0;
	for (unsigned int x = 0; x < RxData[uiIndex].size; x++)
		Avg += RxData[uiIndex].pData[x];

	Avg = Avg / RxData[uiIndex].size;
	return Avg;
}
*/

#include <windows.networking.sockets.h>

#include <iostream>

#include <string>
#include <thread>
#include <vector>

#include "../Shared/Packet.h"

#include "../Shared/Logger.h"
#include "../Shared/configManager.h"

configuration::configManager configurations("../Shared/config.conf");
#ifdef METRICS
int numDataParsesServer = 0;
float maxSizeRxData = 0;

Metrics::Timer timer;
Metrics::Calculations dataParsingTimeCalc;
Metrics::Calculations sizeOfDataParsedDataServerCalc;
Metrics::Calculations sizeOfMemoryServerCalc;
#endif

#pragma comment (lib, "Ws2_32.lib")

const int numColumns = 1;
const int MaxBufferSize = 1000;

struct StorageTypes
{
	std::string startTime = "";
	std::string currentTime = "";
	float startingFuel = 0;
	float sumFuel = 0;
	int size = 0;
};

void UpdateData(StorageTypes* plane, float currentFuelPoint, std::string timeStamp);
float CalcAvg(StorageTypes* plane);
float CalcFuelConsumption(StorageTypes* plane, float currentFuel);

static int numCalc = 0;		//number of calculations
static float calcTime = 0;


void clientHandler(SOCKET clientSocket)
{

	std::thread::id threadID = std::this_thread::get_id();
	unsigned int planeID = *static_cast<unsigned int*>(static_cast<void*>(&threadID));

	std::cout << "Connection Established with Plane: " << planeID << std::endl;

	Packet p(planeID);
	send(clientSocket, p.serialize(), MaxBufferSize, 0);

	StorageTypes plane;
	char RxBuffer[MaxBufferSize] = {}; // magic number
	float fValue;
	std::string timestamp;

	int loopCounter = 0;
	bool exit = false;
	while (!exit)
	{
		memset(RxBuffer, 0, sizeof(RxBuffer));

		size_t result = recv(clientSocket, RxBuffer, sizeof(RxBuffer), 0); // get current value for variable

		p = Packet(RxBuffer);

		if (strcmp(p.getTimestamp().c_str(), "*") == 0) 
		{ 
			p.setCurrentFuelConsumption(CalcFuelConsumption(&plane, fValue));
			p.setAverageFuelConsumption(CalcAvg(&plane));
			p.swapIP();


			std::tm tm1 = {};
			std::istringstream ss1(plane.startTime);
			ss1 >> std::get_time(&tm1, "%d_%m_%Y %H:%M:%S");
			auto timeStart = std::chrono::system_clock::from_time_t(std::mktime(&tm1));

			std::tm tm2 = {};
			std::istringstream ss2(plane.currentTime);
			ss2 >> std::get_time(&tm2, "%d_%m_%Y %H:%M:%S");
			auto timeEnd = std::chrono::system_clock::from_time_t(std::mktime(&tm2));

			auto diff = std::chrono::duration_cast<std::chrono::seconds>(timeEnd - timeStart).count();

			p.setTimeStamp(std::to_string(diff));

			send(clientSocket, p.serialize(), MaxBufferSize, 0); //send final stats back 

			exit = true; 
		} else {

			if (strcmp(p.getParamName().c_str(), configurations.getConfigChar("columnOne")) == 0) {

				fValue = p.getFuelTotalQuantity();
				timestamp = p.getTimestamp();

				if (loopCounter == 0) {
					plane.startingFuel = fValue;
					plane.startTime = timestamp;
				}

				UpdateData(&plane, fValue, timestamp);

				p.setCurrentFuelConsumption(CalcFuelConsumption(&plane, fValue));
				p.setAverageFuelConsumption(CalcAvg(&plane));
				p.swapIP();

				send(clientSocket, p.serialize(), MaxBufferSize, 0);//send average back 

			} else {

				std::string invalidMessage = "Invalid Parameter Name, Closing Connection.";
				// Kill connection if not one of the correct param names
				send(clientSocket, invalidMessage.c_str(), invalidMessage.length(), 0);//send average back 
				exit = true;

			}
		}
		loopCounter++;
	}

	std::cout << "Closing connection to Plane: " << planeID << std::endl;
	closesocket(clientSocket);

}

int main()
{
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return 1;
	}

	// Create a socket for listening
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		std::cerr << "socket failed with error: " << WSAGetLastError() << '\n';
		WSACleanup();
		return 1;
	}

	// Bind the socket to an address and port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(atoi(configurations.getConfigChar("port"))); // Magic Number
	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "bind failed with error: " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Start listening for incoming connections
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "listen failed with error: " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	std::vector<std::thread> threads;

	// Accept incoming connections and spawn a thread to handle each one
	while (true) {
		SOCKET clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "accept failed with error: " << WSAGetLastError() << '\n';
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		threads.emplace_back(clientHandler, clientSocket);
	}

	// Close the listening socket and cleanup Winsock
	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

void UpdateData(StorageTypes* plane, float currentFuelPoint, std::string timeStamp)
{
	plane->size++;
	plane->sumFuel += currentFuelPoint;
	plane->currentTime = timeStamp;
}

float CalcAvg(StorageTypes* plane)
{
	return plane->sumFuel / plane->size;
}

float CalcFuelConsumption(StorageTypes* plane, float currentFuel) {
	return plane->startingFuel - currentFuel;
}
