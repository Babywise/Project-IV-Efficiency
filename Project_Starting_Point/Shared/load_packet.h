#pragma once
#include <string>
const int maxNameLen = 7;
const int maxPortLen = 6;
const int maxIP = 16;
class load_packet {
	struct Header {
		char srcIP[maxIP] = {};
		char destIP[maxIP] = {};
		int seq = 0;
	}head;

	struct data {
		char clientServer[maxNameLen] = {};
		char redirectIP[maxIP] = {};
		char redirectPort[maxPortLen] = {};
		bool terminate = false;
	}payload;
	char* TxBuffer = {};
public:
	/// <summary>
	/// get original packet from client
	/// </summary>
	/// <param name="buffer"></param>
	load_packet(char* buffer) {
		std::memcpy(&this->head, buffer, sizeof(this->head));
		std::memcpy(&this->payload, buffer + sizeof(this->head), sizeof(this->payload));
	}
	/// <summary>
	/// initialize packet to send back to client
	/// </summary>
	/// <param name="clientServer"></param>
	/// <param name="redirect"></param>
	/// <param name="port"></param>
	load_packet(std::string clientServer, std::string redirect, std::string port)
	{
		std::memcpy(&this->payload.clientServer, clientServer.c_str(), clientServer.length());
		std::memcpy(&this->payload.redirectIP, redirect.c_str(),redirect.length());
		std::memcpy(&this->payload.redirectPort, port.c_str(), port.length());
	}
	/// <summary>
/// Serialize function
/// </summary>
/// <returns>char* buffer</returns>
	char* serialize()
	{
		this->head.seq++;

		this->TxBuffer = (char*)malloc(sizeof(head) + sizeof(payload));

		std::memcpy(this->TxBuffer, &this->head, sizeof(this->head));
		std::memcpy(this->TxBuffer + sizeof(this->head), &this->payload, sizeof(this->payload));

		return this->TxBuffer;
	}
	/// <summary>
	/// Retrieves the size of the body contents use for Tx/Rx Buffer
	/// </summary>
	/// <returns>size of packet</returns>
	static int getPacketSize() {
		return sizeof(head) + sizeof(payload);
	}
	/// <summary>
/// Increase Packet Sequence Number by 1
/// </summary>
	void incrementSeq()
	{
		this->head.seq++;
	}
	/// <summary>
	/// Retrieve the current Sequence Number
	/// </summary>
	/// <returns>int SequenceNumber</returns>
	int getSeq() {
		return this->head.seq;
	}
	/// <summary>
	/// Set the Sequence Number
	/// </summary>
	/// <param name="seq"></param>
	void setSeq(int seq) {
		this->head.seq = seq;
	}

	void setClientServer(std::string clientServer) {
		std::memcpy(this->payload.clientServer, clientServer.c_str(), clientServer.length());
	}
	std::string getClientServer() {
		std::string temp = this->payload.clientServer;
		return temp;
	}
	void setRedirectIP(std::string ip) {
		std::memcpy(this->payload.redirectIP, ip.c_str(), ip.length());
	}
	std::string getRedirectIP() {
		std::string temp = this->payload.redirectIP;
		return temp;
	}
	void setRedirectPort(std::string port) {
		std::memcpy(this->payload.redirectPort, port.c_str(), port.length());
	}
	std::string getRedirectPort() {
		std::string temp = this->payload.redirectPort;
		return temp;
	}
	void setTerminate(bool term){
		this->payload.terminate = term;
	}
	bool getTermination() {
		return this->payload.terminate;
	}
};