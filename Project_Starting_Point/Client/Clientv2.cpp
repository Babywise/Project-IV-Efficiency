#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

//include externals
#include <vector>
#include <thread>

//include local stuff
#include "IO.h"
#include "../Shared/Packet.h"

//variables
configuration::configManager configurations("../Shared/config.conf");
const char* lanAddr = configurations.getConfigChar("lanAddr");
const char* wanAddr = configurations.getConfigChar("wanAddr");
const int port = atof(configurations.getConfig("port").c_str());
const std::string wan = configurations.getConfigChar("wan");
const std::string lan = configurations.getConfigChar("lan");

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
	char Rx[128]; // Get from Config File later (Magic Number)



	//startup getSize note. should be started before looking for clients
	std::thread sizeThread(GetSizePromise, std::move(sizeOfFile)); // begin getting size of file
	sizeThread.detach();

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;

	SvrAddr.sin_port = htons(port);

	SvrAddr.sin_addr.s_addr = inet_addr(lanAddr);

	connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)); // connect

	unsigned int countTo = uiSize.get();

	// Loop through number of lines in the file
	for ( unsigned int l = 0; l < countTo; l++ )
	{
		std::string strInput;

		strInput = buffer.next();
		//parse data from file
		
// l != column headers it l is data values

		if ( l > 0 )
		{
			size_t offset, preOffset; // Keeps track of which value to read (param position)
			offset = preOffset = 0;
			unsigned int iParamIndex = 0;
			//while (offset != std::string::npos)

			/*int offset = strInput.find_first_of(',');
			std::string strTx = strInput.substr(0, offset);*/

			// This loop gets the parameter names and values. These are sent to the server where we recieve an acknowledgement
			while ( iParamIndex != atoi(configurations.getConfigChar("paramCount")) )
			{

				offset = strInput.find_first_of(',', preOffset + 1); // Find comma, get size of everything after it
				// Creates a substring for the param name. Uses the offset values to know where to start and end
				std::string strTx = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				//get timer for data parsing

				//------------------------------
				//Packet testing
				Packet pSend("src", "dest", ParamNames[iParamIndex], 5, "20:20", 0.00f);
				pSend.serialize();
				pSend.serialize();
				pSend.serialize();
				pSend.serialize();
				Packet pRecv(pSend.serialize());

				char* TxB = {};

				TxB = pSend.serialize();

				send(ClientSocket, TxB, 1000, 0);
				//-----------------------------

				send(ClientSocket, ParamNames[iParamIndex].c_str(), (int)ParamNames[iParamIndex].length(), 0); // Send parameter name to server
				

				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Ack


				send(ClientSocket, strTx.c_str(), (int)strTx.length(), 0); // Send value to server


				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Average



				std::cout << ParamNames[iParamIndex] << " Avg: " << Rx << std::endl; // Print param name and average
				preOffset = offset; // Update offset to next column
				iParamIndex++; // Increment index of param to read from buffer

			}
		} else
		{

			ParamNames.push_back("TIME STAMP"); // if is index 0 write timestamp
			size_t offset, preOffset;
			offset = 0;
			preOffset = -1;
			while ( offset != std::string::npos )
			{
				offset = strInput.find_first_of(',', preOffset + 1); // find next value after , from the offset ie: offset = 1 get value after second csv (comma-seperated-value)
				std::string newParam = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				ParamNames.push_back(newParam); // All param names
				preOffset = offset;
			}
		}

	}

	send(ClientSocket, "***", 3, 0);
	closesocket(ClientSocket); // cleanup
	WSACleanup();



	return 1;
}





