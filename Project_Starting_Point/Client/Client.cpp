#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

unsigned int GetSize();

int main()
{
	WSADATA wsaData;
	SOCKET ClientSocket;
	sockaddr_in SvrAddr;
	unsigned int uiSize = 0; // uiSize = unsignedInt size?
	vector<string> ParamNames;
	char Rx[128];

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(27001);
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));

	uiSize = GetSize(); 

	// Loop through number of lines in the file
	for (unsigned int l = 0; l < uiSize; l++)
	{
		string strInput;
		ifstream ifs("DataFile.txt");
		for (unsigned int iStart = 0; iStart < l; iStart++)
			getline(ifs, strInput);

		getline(ifs, strInput);
		if (l > 0)
		{
			size_t offset, preOffset; // Keeps track of which value to read (param position)
			offset = preOffset = 0;
			unsigned int iParamIndex = 0;
			//while (offset != std::string::npos)

			// This loop gets the parameter names and values. These are sent to the server where we recieve an acknowledgement
			while(iParamIndex != 8)
			{
				offset = strInput.find_first_of(',', preOffset+1); // Find comma, get size of everything after it
				// Creates a substring for the param name. Uses the offset values to know where to start and end
				string strTx = strInput.substr(preOffset+1, offset - (preOffset+1)); 
				send(ClientSocket, ParamNames[iParamIndex].c_str(), (int)ParamNames[iParamIndex].length(), 0); // Send parameter name to server
				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Ack
				send(ClientSocket, strTx.c_str(), (int)strTx.length(), 0); // Send value to server
				recv(ClientSocket, Rx, sizeof(Rx), 0); // Recieve Average
				cout << ParamNames[iParamIndex] << " Avg: " << Rx << endl; // Print param name and average
				preOffset = offset; // Update offset to next column
				iParamIndex++; // Increment index of param to read from buffer
			}
		}
		// When l is 0 (first iteration of loop)
		else
		{
			ParamNames.push_back("TIME STAMP");
			size_t offset, preOffset;
			offset = 0;
			preOffset = -1;
			while (offset != std::string::npos)
			{
				offset = strInput.find_first_of(',', preOffset + 1);
				string newParam = strInput.substr(preOffset + 1, offset - (preOffset + 1));
				ParamNames.push_back(newParam);
				preOffset = offset;
			}
		}
		ifs.close();
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 1;
}

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