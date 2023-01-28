#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include "../Shared/Metrics.h"
#include "../Shared/Logger.h"

#define WAN
//#define LAN

const char* lanAddr = "127.0.0.1";
const char* wanAddr = "x.x.x.x";
const int port = 27000;
const string wan = "WAN";
const string lan = "LAN";

#define METRICS

#ifdef METRICS
int numDataParsesClient = 0;
Metrics::Calculations calculations;
Metrics::Timer timer;
Metrics::Calculations lineCounter; // line counter used to determine total number of lines used in fileIO
Metrics::Calculations dataParsingTimeCalc;
Metrics::Calculations sizeOfDataParsedDataClientCalc;
#endif

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

#ifdef METRICS
	int numTransmissions = 0;
	int numHandshakes = 0;
	vector<long long> handshakeTimes;
	int handshakeTransmissionCount = 0;
#endif


	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;
	
	SvrAddr.sin_port = htons(port); // Get from Config File later (Magic Number)

#ifdef LAN
	SvrAddr.sin_addr.s_addr = inet_addr(lanAddr); // Get from Config File later (Magic Number)
#endif

#ifdef WAN
	SvrAddr.sin_addr.s_addr = inet_addr(wanAddr); // Get from Config File later (Magic Number)
#endif

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
				
#ifdef METRICS
				std::chrono::time_point<std::chrono::system_clock> start, end;

				start = std::chrono::system_clock::now();
#endif
				send(ClientSocket, ParamNames[iParamIndex].c_str(), (int)ParamNames[iParamIndex].length(), 0); // Send parameter name to server
#ifdef METRICS
				numTransmissions++;
#endif

				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Ack
#ifdef METRICS
				numTransmissions++;
#endif

				send(ClientSocket, strTx.c_str(), (int)strTx.length(), 0); // Send value to server
#ifdef METRICS
				numTransmissions++;
#endif

				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Average
#ifdef METRICS
				numTransmissions++;
				numHandshakes++;

				end = std::chrono::system_clock::now();
				std::chrono::duration<double> elapsedTimeSeconds = end - start;
				auto elapsedTimeMicSec = std::chrono::duration_cast<std::chrono::microseconds>(elapsedTimeSeconds).count();
				handshakeTimes.push_back(elapsedTimeMicSec);
				//cout << "HandshakeTime: " << elapsedTimeMicSec << endl;

				if (numHandshakes == 1) {
					handshakeTransmissionCount = numTransmissions;
				}
#endif
				

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
#ifdef METRICS
				sizeOfDataParsedDataClientCalc.addPoint(newParam.length());
			}
			//get timer for data parsing
			dataParsingTimeCalc.addPoint(timer.getTime());
			numDataParsesClient++;
#endif
		}
		ifs.close(); // close file
	}
#ifdef METRICS
	timer.start();
	GetSize();
	Metrics::logIOMetrics(calculations, lineCounter, timer.getTime());
	Metrics::logDataParsingMetricsClient(dataParsingTimeCalc, sizeOfDataParsedDataClientCalc, numDataParsesClient);
#endif
	closesocket(ClientSocket); // cleanup
	WSACleanup();

#ifdef METRICS

	int avgHandshake = 0;
	for (int i = 0; i < handshakeTimes.size(); i++) {
		avgHandshake += handshakeTimes.at(i);
	}

	avgHandshake = avgHandshake / handshakeTimes.size();

#ifdef WAN
	Metrics::logNetworkMetricsClient(numTransmissions, avgHandshake, handshakeTransmissionCount, wan);
#endif // WAN
#ifdef LAN
	Metrics::logNetworkMetricsClient(numTransmissions, avgHandshake, handshakeTransmissionCount, lan);
#endif // LAN

#endif


	return 1;
}

/// <summary>
/// gets the size of the file DataFile.txt
/// Note. should run before looking for clients perhaps in a thread
/// </summary>
/// <returns>uiSize (number of lines in the file)</returns>
unsigned int GetSize()

{
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