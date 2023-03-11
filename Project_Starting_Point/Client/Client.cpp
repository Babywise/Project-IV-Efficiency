#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

//include externals
#include <vector>
#include <thread>

//include local stuff
#include "IO.h"
#include "../Shared/Packet.h"

// defines
//#define WAN
#define LAN
#define METRICS


//variables
configuration::configManager configurations("../Shared/config.conf");
const char* lanAddr = configurations.getConfigChar("lanAddr");
const char* wanAddr = configurations.getConfigChar("wanAddr");
const int port = atof(configurations.getConfig("port").c_str());
const std::string wan = configurations.getConfigChar("wan");
const std::string lan = configurations.getConfigChar("lan");
const int MaxBufferSize = 1000;


//metrics variables
#ifdef METRICS
int numDataParsesClient = 0;
Metrics::Calculations calculations;
Metrics::Timer timer;
Metrics::Calculations lineCounter; // line counter used to determine total number of lines used in fileIO
Metrics::Calculations dataParsingTimeCalc;
Metrics::Calculations sizeOfDataParsedDataClientCalc;
float logTime; // used to measure getSize since it has been refactored for future and promise
#endif


/// <summary>
/// main loop 
/// </summary>
/// <returns></returns>
int main(int argc, char* argv[])
{
	fileIO::fileBuffer buffer(configurations.getConfigChar("dataFile"));


	//setup
	WSADATA wsaData;
	SOCKET ClientSocket;
	sockaddr_in SvrAddr;
	std::promise<unsigned int> sizeOfFile; // used to begin getting size of file while starting up server
	std::future<unsigned int> uiSize = sizeOfFile.get_future();
	std::vector<std::string> ParamNames;
	char Rx[MaxBufferSize]; // Get from Config File later (Magic Number)

#ifdef METRICS
	int numTransmissions = 0;
	int numHandshakes = 0;
	std::vector<long long> handshakeTimes;
	int handshakeTransmissionCount = 0;
#endif

	//startup getSize note. should be started before looking for clients
	std::thread sizeThread(GetSizePromise, std::move(sizeOfFile)); // begin getting size of file
	sizeThread.detach();

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;
	
	SvrAddr.sin_port = htons(port); 

#ifdef LAN
	SvrAddr.sin_addr.s_addr = inet_addr(lanAddr); 
#endif

#ifdef WAN
	SvrAddr.sin_addr.s_addr = inet_addr(wanAddr); 
#endif

	connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)); // connect

#ifdef METRICS
	timer.start();
	unsigned int countTo = uiSize.get();
	logTime = timer.getTime();
#else
	unsigned int countTo = uiSize.get();
#endif

	memset(Rx, 0, sizeof(Rx));
	recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve PlaneID

#ifdef METRICS
	numTransmissions++;
#endif

	Packet plane(Rx);

	std::string paramName;
	std::string timestamp;
	std::string fuelValue;
	float startingFuel;

	int offset, preOffset = 0;

	// Loop through number of lines in the file
	for (unsigned int l = 0; l < countTo; l++)
	{
		std::string strInput;
#ifdef METRICS
		timer.start();
#endif
		strInput = buffer.next();

#ifdef METRICS
		calculations.addPoint(timer.getTime());
		lineCounter.addPoint(1); // add 1 for the get line above, add one for close file at end of loop add one for file init
#endif

		
		if (l == 0)
		{
			int endpos = strInput.find_first_of(',');
			paramName = strInput.substr(0, endpos);
		}

		if (l > 0)
		{
			size_t offset, preOffset;
			offset = preOffset = 0;

			for (int pCounter = 0; pCounter < 2; pCounter++) {
				offset = strInput.find_first_of(',', preOffset + 1);
				if (pCounter == 0) {
					timestamp = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				} else {
					if (l == 1) {
						startingFuel = atof(strInput.substr(preOffset + 1, offset - (preOffset + 1)).c_str());
					}
					fuelValue = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				}
				preOffset = offset; // Update offset to next column
			}
#ifdef LAN
			plane = Packet("src", lanAddr, paramName, plane.getPlaneID(), timestamp, atof(fuelValue.c_str()));
#endif
#ifdef WAN
			plane = Packet("src", wanAddr, paramName, plane.getPlaneID(), timestamp, atof(fuelValue.c_str()));
#endif

#ifdef METRICS
			std::chrono::time_point<std::chrono::system_clock> start, end;
			start = std::chrono::system_clock::now();
#endif

			send(ClientSocket, plane.serialize(), MaxBufferSize, 0); // Send parameter name to server
			size_t result = recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve PlaneID

#ifdef METRICS
			numTransmissions++;
			numTransmissions++;
			numHandshakes++;
			if (l == 1) { handshakeTransmissionCount = numTransmissions - 1; }
			end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsedTimeSeconds = end - start;
			auto elapsedTimeMicSec = std::chrono::duration_cast<std::chrono::microseconds>(elapsedTimeSeconds).count();
			handshakeTimes.push_back(elapsedTimeMicSec);
			//cout << "HandshakeTime: " << elapsedTimeMicSec << endl;

#endif

			if (result == SOCKET_ERROR) {

				DWORD err = GetLastError();
				LPSTR messageBuffer = nullptr;

				size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

				std::string errMessage(messageBuffer, size);
				std::cout << errMessage << std::endl;

				break;

			} else {

				plane = Packet(Rx);

				if (l < countTo - 1)
				{
					std::cout << "Timestamp: " << plane.getTimestamp() << " | Fuel Consumption: " << plane.getCurrentFuelConsumption() <<
						" | Average Fuel Consumption: " << plane.getAverageFuelConsumption() << std::endl;
				} else
				{
					std::cout << std::endl << "Flight Statistics: " << std::endl;
					std::cout << "Flight ID: " << plane.getPlaneID() << std::endl;
					std::cout << "Flight Duration (seconds): " << plane.getTimestamp() << std::endl;
					std::cout << "Flight Starting Fuel: " << startingFuel << std::endl;
					std::cout << "Flight Ending Fuel: " << startingFuel - plane.getCurrentFuelConsumption() << std::endl;
					std::cout << "Flight Total Fuel Consumption: " << plane.getCurrentFuelConsumption() << std::endl;
					std::cout << "Flight Average Fuel: " << plane.getAverageFuelConsumption() << std::endl << std::endl;
				}
			}
		}
	}

#ifdef METRICS
	timer.start();
	Metrics::logStartOfClient(configurations.getConfigChar("dataFile"), plane.getPlaneID());
	Metrics::logSystemStatsMetrics(true);
	Metrics::logClientIOMetrics(calculations, lineCounter, logTime);
	Metrics::logDataParsingMetricsClient(dataParsingTimeCalc, sizeOfDataParsedDataClientCalc, numDataParsesClient);
#endif

	closesocket(ClientSocket); // cleanup
	WSACleanup();

	system("pause");


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
	Metrics::addLogEndOfFileSpacing(true);
#endif

	return 1;
}