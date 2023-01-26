#include <windows.networking.sockets.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

#include "../Shared/Metrics.h"
#include "../Shared/Logger.h"

#define METRICS
/*
* When in calculating metrics mode... Set above
*/
#ifdef METRICS
Logger logger;
Metrics::Timer timer;
Metrics::Calculations dataParsingCalc;
#endif

using namespace std;

struct StorageTypes 
{ 
	unsigned int size = 0;
	float* pData;
};
StorageTypes RxData[7];

void UpdateData(unsigned int, float);
float CalcAvg(unsigned int);

/// <summary>
/// main loop that only accepts a single client
/// </summary>
/// <returns></returns>
int main()
{
#ifdef METRICS
	logger.log("Server Started","ServerDataParsingMetrics");
#endif

	//setup
	WSADATA wsaData;
	SOCKET ServerSocket, ConnectionSocket;
	char RxBuffer[128] = {}; // magic number
	sockaddr_in SvrAddr;

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == SOCKET_ERROR)
		return -1;

	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = INADDR_ANY;
	SvrAddr.sin_port = htons(27001); // Magic Number
	bind(ServerSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));

	if (ServerSocket == SOCKET_ERROR)
		return -1;
	// accepts a single connection
	listen(ServerSocket, 1);
	cout << "Waiting for client connection\n" << endl;
	ConnectionSocket = SOCKET_ERROR;
	ConnectionSocket = accept(ServerSocket, NULL, NULL);

	if (ConnectionSocket == SOCKET_ERROR)
		return -1;

	cout << "Connection Established" << endl;

	// if first byte of buffer is an asterisk close connection
	while (RxBuffer[0] != '*')
	{
		float fValue = 0;
		memset(RxBuffer, 0, sizeof(RxBuffer));
		recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0); // get param name
		send(ConnectionSocket, "ACK", sizeof("ACK"), 0); // send ack

		// find param name recieved
		if (strcmp(RxBuffer, "ACCELERATION BODY X") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0); // get current value for variable
			fValue = (float)atof(RxBuffer);
			UpdateData(0, fValue);// add data to list
			fValue = CalcAvg(0);// calculate the average | fValue sends at end of elif
		}
		else if (strcmp(RxBuffer, "ACCELERATION BODY Y") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(1, fValue);
			fValue = CalcAvg(1);
		}
		else if (strcmp(RxBuffer, "ACCELERATION BODY Z") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(2, fValue);
			fValue = CalcAvg(2);
		}
		else if (strcmp(RxBuffer, "TOTAL WEIGHT") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(3, fValue);
			fValue = CalcAvg(3);
		}
		else if (strcmp(RxBuffer, "PLANE ALTITUDE") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(4, fValue);
			fValue = CalcAvg(4);
		}
		else if (strcmp(RxBuffer, "ATTITUDE INDICATOR PICTH DEGREES") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(5, fValue);
			fValue = CalcAvg(5);
		}
		else if (strcmp(RxBuffer, "ATTITUDE INDICATOR BANK DEGREES") == 0)
		{
			memset(RxBuffer, 0, sizeof(RxBuffer));
			size_t result = recv(ConnectionSocket, RxBuffer, sizeof(RxBuffer), 0);
			fValue = (float)atof(RxBuffer);
			UpdateData(6, fValue);
			fValue = CalcAvg(6);
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

	closesocket(ConnectionSocket);	//closes incoming socket
	closesocket(ServerSocket);	    //closes server socket	
	WSACleanup();					//frees Winsock resources

#ifdef METRICS
	//data parsing results
	logger.log("Server - DataParsing - Sum = " + to_string(dataParsingCalc.getSum()), "ServerDataParsingMetrics");
	logger.log("Server - DataParsing - Average = " + to_string(dataParsingCalc.getAverage()), "ServerDataParsingMetrics");
#endif

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

		pNewData[RxData[uiIndex].size] = value;
		delete[] RxData[uiIndex].pData; // delete old memory
		RxData[uiIndex].pData = pNewData; // replace with new data added
		RxData[uiIndex].size++;
	}
#ifdef METRICS
	dataParsingCalc.addPoint(timer.getTime());
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
	for (unsigned int x = 0; x < RxData[uiIndex].size; x++)
		Avg += RxData[uiIndex].pData[x];

	Avg = Avg / RxData[uiIndex].size;
	return Avg;
}