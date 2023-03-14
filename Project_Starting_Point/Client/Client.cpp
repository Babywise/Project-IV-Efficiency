#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

//include externals
#include <vector>
#include <thread>

//include local stuff
#include "IO.h"
#include "../Shared/Packet.h"
#include "../Shared/load_packet.h"

// defines
//#define WAN
#define LAN
//#define METRICS

//variables
configuration::configManager configurations("../Shared/config.conf");
char* lanAddr = configurations.getConfigChar("lanAddr");
const char* wanAddr = configurations.getConfigChar("wanAddr");
int port = atof(configurations.getConfig("port").c_str());
const std::string wan = configurations.getConfigChar("wan");
const std::string lan = configurations.getConfigChar("lan");
void setup();
//metrics variables
#ifdef METRICS
int numDataParsesClient = 0;
Metrics::Calculations IOCalculations;
Metrics::Timer timer;
Metrics::Calculations lineCounter; // line counter used to determine total number of lines used in fileIO
Metrics::Calculations dataParsingTimeCalc; //parse time calculations
Metrics::Calculations sizeOfDataParsedDataClientCalc; //size of parsed data
float logTime; // used to measure getSize since it has been refactored for future and promise
#endif


/// <summary>
/// main loop 
/// </summary>
/// <returns></returns>
int main(int argc, char* argv[])
{
	setup();
	//setup
	WSADATA wsaData;
	SOCKET ClientSocket;
	sockaddr_in SvrAddr;
	std::promise<unsigned int> sizeOfFile; // used to begin getting size of file while starting up server
	std::future<unsigned int> uiSize = sizeOfFile.get_future();
	std::vector<std::string> ParamNames;
	char* Rx = (char*)malloc(Packet::getPacketSize());

#ifdef METRICS
	int numTransmissions = 0;
	int numHandshakes = 0;
	std::vector<long long> handshakeTimes;
	int handshakeTransmissionCount = 0;
#endif
	//startup getSize note. should be started before looking for clients
	std::thread sizeThread(GetSizePromise, std::move(sizeOfFile)); // begin getting size of file
	sizeThread.detach();
	fileIO::fileBuffer* buffer = new fileIO::fileBuffer(configurations.getConfigChar("dataFile"), "../Shared/config.conf"); // begin buffering file
	

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
	std::cout << "connecting to " << lanAddr << ":" << port << std::endl;
	connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)); // connect

#ifdef METRICS
	timer.start();
	unsigned int countTo = uiSize.get();
	logTime = timer.getTime();
#else
	unsigned int countTo = uiSize.get();
#endif

	memset(Rx, NULL, Packet::getPacketSize());
	recv(ClientSocket, Rx, Packet::getPacketSize(), 0); // Recieve PlaneID

#ifdef METRICS
	numTransmissions++;
	timer.start();
#endif

	Packet plane(Rx);

#ifdef METRICS
	numDataParsesClient += 3; 
	dataParsingTimeCalc.addPoint(timer.getTime());
	sizeOfDataParsedDataClientCalc.addPoint(Packet::getPacketSize());
#endif

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
		
		strInput = buffer->next();

#ifdef METRICS
		IOCalculations.addPoint(timer.getTime());
		lineCounter.addPoint(1); // add 1 for the get line above, add one for close file at end of loop add one for file init
		timer.start();
#endif

		if (l == 0)
		{
			int endpos = strInput.find_first_of(',');
			paramName = strInput.substr(0, endpos);
#ifdef METRICS
			dataParsingTimeCalc.addPoint(timer.getTime());
			numDataParsesClient++;
			sizeOfDataParsedDataClientCalc.addPoint(sizeof(paramName));
#endif
		}

#ifdef METRICS
		timer.start();
#endif
		if (l > 0)
		{
			size_t offset, preOffset;
			offset = preOffset = 0;

			for (int pCounter = 0; pCounter < 2; pCounter++) {
				offset = strInput.find_first_of(',', preOffset + 1);
				if (pCounter == 0) {
					timestamp = strInput.substr(preOffset + 1, offset - (preOffset + 1));
#ifdef METRICS
					dataParsingTimeCalc.addPoint(timer.getTime());
					numDataParsesClient++;
					sizeOfDataParsedDataClientCalc.addPoint(sizeof(timestamp));
#endif

				} else {
					if (l == 1) {
						startingFuel = atof(strInput.substr(preOffset + 1, offset - (preOffset + 1)).c_str());
#ifdef METRICS
						dataParsingTimeCalc.addPoint(timer.getTime());
						numDataParsesClient++;
						sizeOfDataParsedDataClientCalc.addPoint(sizeof(startingFuel));
#endif
					}
				
					fuelValue = strInput.substr(preOffset + 1, offset - (preOffset + 1));
#ifdef METRICS
					dataParsingTimeCalc.addPoint(timer.getTime());
					numDataParsesClient++;
					sizeOfDataParsedDataClientCalc.addPoint(sizeof(fuelValue));
#endif
					
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

			send(ClientSocket, plane.serialize(), Packet::getPacketSize(), 0); // Send parameter name to server
#ifdef METRICS
			numDataParsesClient += 3;
			sizeOfDataParsedDataClientCalc.addPoint(Packet::getPacketSize());
#endif
			size_t result = recv(ClientSocket, Rx, Packet::getPacketSize(), 0); // Recieve PlaneID

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
#ifdef METRICS
				timer.start();
#endif
				plane = Packet(Rx);
#ifdef METRICS
				dataParsingTimeCalc.addPoint(timer.getTime());
				numDataParsesClient += 3;
				sizeOfDataParsedDataClientCalc.addPoint(Packet::getPacketSize());
#endif
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
	Metrics::setClientLogName(configurations.getConfigChar("clientMetricsLogFileName") + std::string(" - ") + std::to_string(plane.getPlaneID()));
	Metrics::logStartOfClient(configurations.getConfigChar("dataFile"), plane.getPlaneID());
	Metrics::logSystemStatsMetrics(true);
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
	Metrics::logClientIOMetrics(IOCalculations, lineCounter, logTime);
	Metrics::addLogEndOfFileSpacing(true);
#endif

	return 1;
}



void setup() {
	//setup
	WSADATA wsaData;
	SOCKET ClientSocket;
	sockaddr_in SvrAddr;
	char* Rx = (char*)malloc(4);




	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;
	int balanceport = atoi(configurations.getConfigChar("balancer_port"));
	SvrAddr.sin_port = htons(balanceport);

	SvrAddr.sin_addr.s_addr = inet_addr(configurations.getConfigChar("balancer_IP"));
	std::cout << "Connecting";
	int v = connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)); // connect
	std::cout << "Connected";
	recv(ClientSocket, Rx, 3, 0);
	Rx[3] = '\0';
	if (strcmp(Rx, "ACK") == 0) {
		std::cout << "got ACK";
		load_packet pack("client","","");
		send(ClientSocket, pack.serialize(), load_packet::getPacketSize(), 0);
	}
	Rx = (char*)malloc(load_packet::getPacketSize());
	recv(ClientSocket, Rx, load_packet::getPacketSize(), 0);
	load_packet rcv_pack(Rx);
	if (strcmp(rcv_pack.getRedirectIP().c_str(), "0.0.0.0") != 0) {
		lanAddr = (char*)malloc(strlen(rcv_pack.getRedirectIP().c_str())*sizeof(char) + 1);
		strcpy_s(lanAddr,strlen(rcv_pack.getRedirectIP().c_str())+1, rcv_pack.getRedirectIP().c_str());
		port = atoi(rcv_pack.getRedirectPort().c_str());
	}
	std::cout << "\ngot address : " << lanAddr << ":" << port << std::endl;
	std::cout << "Closing";
	closesocket(ClientSocket);
}