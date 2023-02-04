#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>

#include "../Shared/Metrics.h"
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

using namespace std;

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
	char RxBuffer[128] = {}; // magic number
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

	cout << "Waiting for client connection\n" << endl;
	ConnectionSocket = SOCKET_ERROR;
	ConnectionSocket = accept(ServerSocket, NULL, NULL);

	if (ConnectionSocket == SOCKET_ERROR)
		return -1;

	int numConnections = 1;

	cout << "Connection Established" << endl;

	// if first byte of buffer is an asterisk close connection
	while (RxBuffer[0] != configurations.getConfigChar("terminator")[0])
	{
		float fValue = 0;
		memset(RxBuffer, 0, sizeof(RxBuffer));
		recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0); // get param name
		send(ConnectionSocket, "ACK", sizeof("ACK"), 0); // send ack

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
	Metrics::addLogEndOfFileSpacing();
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
#ifdef METRICS
	//start timer for data parsing
	timer.start();
#endif
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
#ifdef METRICS
		sizeOfDataParsedDataServerCalc.addPoint(RxData[uiIndex].size);
#endif
		pNewData[RxData[uiIndex].size] = value;
		delete[] RxData[uiIndex].pData; // delete old memory
		RxData[uiIndex].pData = pNewData; // replace with new data added
		RxData[uiIndex].size++;
	}
#ifdef METRICS
	dataParsingTimeCalc.addPoint(timer.getTime());
	numDataParsesServer++;
#endif
}

/// <summary>
/// calculates the average for whatever index is passed in ie 1 = body x 2 = body y
/// </summary>
/// <param name="uiIndex"></param>
/// <returns></returns>
float CalcAvg(unsigned int uiIndex)
{
	float Avg = 0;
	for (unsigned int x = 0; x < RxData[uiIndex].size; x++) {
		Avg += RxData[uiIndex].pData[x];
		numCalc += 1;
	}

	Avg = Avg / RxData[uiIndex].size;
	numCalc += 1;
	return Avg;
}