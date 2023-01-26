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
const string DPCltMetrics = "ClientDataParsingMetrics";
int numDataParsesClient = 0;
Metrics::Calculations calculations;
Logger logger;
Metrics::Timer timer;
Metrics::Calculations lineCounter; // line counter used to determine total number of lines used in fileIO
Metrics::Calculations dataParsingTimeCalc;
Metrics::Calculations sizeOfDataParsedDataClientCalc;
#endif
void logSystemInfo();
#include <filesystem>
using namespace std;
unsigned int GetSize();
/// <summary>
/// main loop 
/// </summary>
/// <returns></returns>
int main(int argc, char* argv[])
{

	//setup
	WSADATA wsaData;
	SOCKET ClientSocket;
	sockaddr_in SvrAddr;
	unsigned int uiSize = 0;
	vector<string> ParamNames;
	char Rx[128]; // Get from Config File later (Magic Number)

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(27001); // Get from Config File later (Magic Number)
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Get from Config File later (Magic Number)
	connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)); // connect

	uiSize = GetSize(); // Gets the total number of lines from the file

	// Loop through number of lines in the file
	for (unsigned int l = 0; l < uiSize; l++)
	{
		string strInput;
#ifdef METRICS
		timer.start();
#endif
		ifstream ifs("DataFile.txt"); // open datafile.txt | file opens on every loop execution
		for (unsigned int iStart = 0; iStart < l; iStart++) {// get next line which is L
			getline(ifs, strInput);
		}
		getline(ifs, strInput);
#ifdef METRICS
		//----should i add a data parse counter here?
		calculations.addPoint(timer.getTime());
		lineCounter.addPoint(3); // add 1 for the get line above, add one for close file at end of loop add one for file init
		lineCounter.addPoint(2*l); // add 2 lines for every loop of the for loop above
#endif
// l != column headers it l is data values
		if (l > 0)
		{
			size_t offset, preOffset; // Keeps track of which value to read (param position)
			offset = preOffset = 0;
			unsigned int iParamIndex = 0;
			//while (offset != std::string::npos)


			// This loop gets the parameter names and values. These are sent to the server where we recieve an acknowledgement
			while(iParamIndex != 8)
			{
#ifdef METRICS
				//start timer for data parsing
				timer.start();
#endif
				offset = strInput.find_first_of(',', preOffset+1); // Find comma, get size of everything after it
				// Creates a substring for the param name. Uses the offset values to know where to start and end
				string strTx = strInput.substr(preOffset+1, offset - (preOffset+1)); 
				//get timer for data parsing
#ifdef METRICS
				sizeOfDataParsedDataClientCalc.addPoint(strTx.length());
				dataParsingTimeCalc.addPoint(timer.getTime());
				numDataParsesClient++;
#endif
				send(ClientSocket, ParamNames[iParamIndex].c_str(), (int)ParamNames[iParamIndex].length(), 0); // Send parameter name to server
				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Ack
				send(ClientSocket, strTx.c_str(), (int)strTx.length(), 0); // Send value to server
				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Average
				cout << ParamNames[iParamIndex] << " Avg: " << Rx << endl; // Print param name and average
				preOffset = offset; // Update offset to next column
				iParamIndex++; // Increment index of param to read from buffer

			}
		}
		else
		{
#ifdef METRICS
			//start timer for data parsing
			timer.start();
#endif
			ParamNames.push_back("TIME STAMP"); // if is index 0 write timestamp
			size_t offset, preOffset;
			offset = 0;
			preOffset = -1;
			while (offset != std::string::npos)
			{
				offset = strInput.find_first_of(',', preOffset + 1); // find next value after , from the offset ie: offset = 1 get value after second csv (comma-seperated-value)
				string newParam = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				ParamNames.push_back(newParam); // All param names
				preOffset = offset;
			}
#ifdef METRICS
			//get timer for data parsing
			dataParsingTimeCalc.addPoint(timer.getTime());
			numDataParsesClient++;
#endif
		}
		ifs.close(); // close file
	}
	logSystemInfo();
	closesocket(ClientSocket); // cleanup
	WSACleanup();

	return 1;
}

/// <summary>
/// gets the size of the file DataFile.txt
/// Note. should run before looking for clients perhaps in a thread
/// </summary>
/// <returns>uiSize (number of lines in the file)</returns>
unsigned int GetSize()

{
	//----potential overlap timer with fileio?
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

	return uiSize;
}

void logSystemInfo() {
#ifdef METRICS
	logger.emptyLine("metrics"); // write system information to lof before start of metrics logging
	logger.log("------------------------------ Start of metrics run -------------------------", "metrics");
	logger.emptyLine("metrics");
	system("wmic cpu get CurrentClockSpeed, MaxClockSpeed, Name, CurrentVoltage, DataWidth, ProcessorType >> %cd%/Logs/metrics.log");
	logger.emptyLine("metrics");
	system("wmic memorychip get FormFactor, Speed, Capacity, DataWidth, Manufacturer, name >> %cd%/Logs/metrics.log");
	logger.emptyLine("metrics");
	system("wmic diskdrive get manufacturer, size,name, model, description >> %cd%/Logs/metrics.log");
	logger.emptyLine("metrics");
	timer.start();
	GetSize();
	logger.log("Get File Size :" + to_string((timer.getTime())) + "ms", "metrics");
	logger.log("Average time to get line from file : " + to_string(calculations.getAverage()) + "ms", "metrics");
	logger.log("TotalTime reading files to get specific lines : " + to_string(calculations.getSum()) + "ms", "metrics");
	logger.log("Total lines reading files ( not including get file length ) : " + to_string(lineCounter.getSum()), "metrics");
	logger.emptyLine("metrics");
	logger.log("------------------------------ End of metrics run -------------------------", "metrics");
#endif
#ifdef METRICS
	//data parsing results
	logger.log("Client Started", DPCltMetrics);
	logger.log("Client - DataParsing - Sum = " + to_string(dataParsingTimeCalc.getSum()) + " ms", DPCltMetrics);
	logger.log("Client - DataParsing - Average = " + to_string(dataParsingTimeCalc.getAverage()) + " ms", DPCltMetrics);
	logger.log("Client - DataParsing - # of Conversions = " + to_string(numDataParsesClient), DPCltMetrics);
	logger.log("Client - DataParsing - Input Size of Parsed Data = ----- Bytes", DPCltMetrics);
	logger.log("Client - DataParsing - Total Size of Parsed Data = " + to_string((int)sizeOfDataParsedDataClientCalc.getSum()) + " Bytes", DPCltMetrics);
	logger.emptyLine(DPCltMetrics);
#endif
}