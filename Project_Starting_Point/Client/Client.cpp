#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../Shared/Metrics.h"
#include "../Shared/Logger.h"

#define METRICS
/*
* When in calculating metrics mode... Set above
*/
#ifdef METRICS
Metrics::Calculations calculations;
Logger logger;
Metrics::Timer timer;
#endif

using namespace std;
unsigned int GetSize();
/// <summary>
/// main loop 
/// </summary>
/// <returns></returns>
int main()
{
	//setup
	WSADATA wsaData;
	SOCKET ClientSocket;
	sockaddr_in SvrAddr;
	unsigned int uiSize = 0;
	vector<string> ParamNames;
	char Rx[128];

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(27001);
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)); // connect

	uiSize = GetSize(); // get until end of file
	for (unsigned int l = 0; l < uiSize; l++)
	{
		string strInput;
		#ifdef METRICS
		timer.start();
		#endif
		ifstream ifs("DataFile.txt"); // open datafile.txt | file opens on every loop execution
		for (unsigned int iStart = 0; iStart < l; iStart++) // get next line which is L
			getline(ifs, strInput);

		getline(ifs, strInput);
		#ifdef METRICS
		calculations.addPoint(timer.getTime());
		#endif
		// l != column headers it l is data values
		if (l > 0)
		{
			size_t offset, preOffset;
			offset = preOffset = 0;
			unsigned int iParamIndex = 0;
			//while (offset != std::string::npos)
			while(iParamIndex != 8) // for all headers
			{
				offset = strInput.find_first_of(',', preOffset+1);
				string strTx = strInput.substr(preOffset+1, offset - (preOffset+1));
				send(ClientSocket, ParamNames[iParamIndex].c_str(), (int)ParamNames[iParamIndex].length(), 0); // send type
				recv(ClientSocket, Rx, sizeof(Rx), 0);// get ack
				send(ClientSocket, strTx.c_str(), (int)strTx.length(), 0); // send current value
				recv(ClientSocket, Rx, sizeof(Rx), 0); // recv avg from server
				cout << ParamNames[iParamIndex] << " Avg: " << Rx << endl; //write average
				preOffset = offset;
				iParamIndex++; // move to next variable type
			}
		}
		else // if zero index ie: column names
		{
			ParamNames.push_back("TIME STAMP"); // if is index 0 write timestamp
			size_t offset, preOffset;
			offset = 0;
			preOffset = -1;
			while (offset != std::string::npos)
			{
				offset = strInput.find_first_of(',', preOffset + 1); // find next value after , from the offset ie: offset = 1 get value after second csv
				string newParam = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				ParamNames.push_back(newParam); // set param names to next paramater
				preOffset = offset;
			}
		}
		ifs.close(); // close file
	}

	closesocket(ClientSocket); // cleanup
	WSACleanup();
#ifdef METRICS
	logger.log("Average time to get line from file : " + to_string(calculations.getAverage())+"ms","metrics");
	logger.log("TotalTime reading files to get specific lines : " + to_string(calculations.getSum())+"ms","metrics");
#endif
	return 1;
}

/// <summary>
/// gets the size of the file DataFile.txt
/// Note. should run before looking for clients perhaps in a thread
/// </summary>
/// <returns></returns>
unsigned int GetSize()
{ 
#ifdef METRICS
	timer.start();
#endif
	// ANCHOR 
	string strInput;
	unsigned int uiSize = 0;
	ifstream ifs("DataFile.txt");
	if (ifs.is_open())
	{
		while (!ifs.eof())
		{
			getline(ifs, strInput);
			uiSize++;
		}
	}
#ifdef METRICS
	logger.log("Get File Size :"+to_string((timer.getTime()))+"ms","metrics");
#endif
	return uiSize;
}