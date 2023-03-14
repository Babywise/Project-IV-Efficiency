#include <windows.networking.sockets.h>
#pragma comment (lib, "Ws2_32.lib")
#include <iostream>

#include <string>
#include <thread>
#include <vector>
#include "../Shared/Logger.h"
#include "../Shared/configManager.h"
#include "../Shared/load_packet.h"
configuration::configManager configurations("../Shared/config.conf");
std::vector<std::string> hosts;
std::vector<std::string> ports;
std::mutex lock;
int index = 0;
struct SERVER {
	std::string ip;
	std::string port;
};
void handleServer(SOCKET sock, load_packet packet);
void handleClient(SOCKET sock, load_packet packet);


void RouteTraffic(SOCKET sock) {
	std::cout << "SEND ACK";
	send(sock, (char*)"ACK", 3, 0);
	char* RxBuffer = (char*)malloc(load_packet::getPacketSize());
	memset(RxBuffer, NULL, load_packet::getPacketSize());
	recv(sock, RxBuffer, load_packet::getPacketSize(), 0);
	std::cout << "Recieved";
	load_packet pack(RxBuffer);
	std::string type = pack.getClientServer();
	if (strcmp(type.c_str(), "client") == 0) {
		handleClient(sock, pack);
	}
	else if (strcmp(type.c_str(), "server") == 0) {
		handleServer(sock, pack);
	}

}
SERVER getNextServer() {
	lock.lock();
	SERVER temp;
	temp.ip = hosts.at(index);
	temp.port = ports.at(index);
	index++;
	if (index >= hosts.size()) { // reset to beginning of list for round robin
		index = 0;
	}
	lock.unlock();
	std::cout << "Connecting to server : " << temp.ip << ":" << temp.port<<std::endl;
	return temp;
}
void handleServer(SOCKET sock, load_packet packet) {
	if (packet.getTermination() == false) {
		closesocket(sock);
		lock.lock();
		std::cout << "Adding server : " << packet.getRedirectIP() << ":" << packet.getRedirectPort() << std::endl;
		hosts.push_back(packet.getRedirectIP());
		ports.push_back(packet.getRedirectPort());
		lock.unlock();
	}
	else {
		if (hosts.size() > 0) {
			int iterator = 0;
			for (int i = 0; i < hosts.size(); i++) {
				if (hosts.at(i) == packet.getRedirectIP()) {
					iterator = i;
					break;
				}
			}

			hosts.erase(hosts.begin() + iterator);
			ports.erase(ports.begin() + iterator);
		}
	}
}
void handleClient(SOCKET sock,load_packet packet) {
	if (hosts.size() > 0) { // 1 or more servers to connect to
		SERVER data = getNextServer();
		packet.setRedirectIP(data.ip);
		packet.setRedirectPort(data.port);
	}
	else { // no available servers
		packet.setRedirectIP("0.0.0.0");
		packet.setRedirectPort("0");
	}
	send(sock, packet.serialize(), load_packet::getPacketSize(), 0);
	closesocket(sock);
}
int main()
{

	std::cout << "Initializing...\n";
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return 1;
	}
	std::cout << "setup listening socket...\n";
	// Create a socket for listening
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		std::cerr << "socket failed with error: " << WSAGetLastError() << '\n';
		WSACleanup();
		return 1;
	}
	else {

		std::cout << "Binding...\n";
		// Bind the socket to an address and port
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(atoi(configurations.getConfigChar("balancer_port"))); // Magic Number
		if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			std::cerr << "bind failed with error: " << WSAGetLastError() << '\n';
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		std::cout << "Listening...\n";
		// Start listening for incoming connections
		if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
			std::cerr << "listen failed with error: " << WSAGetLastError() << '\n';
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		std::vector<std::thread> threads;

		// Accept incoming connections and spawn a thread to handle each client
		while (true) {
			SOCKET clientSocket = accept(listenSocket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
				std::cerr << "accept failed with error: " << WSAGetLastError() << '\n';
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}

			DWORD timeout = atoi(configurations.getConfigChar("sockettimeoutseconds")) * 1000;
			setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

			threads.emplace_back(RouteTraffic, clientSocket);
		}
	}
	// Close the listening socket and cleanup Winsock
	closesocket(listenSocket);
	WSACleanup();
}