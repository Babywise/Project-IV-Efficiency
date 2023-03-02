#pragma once
#include <string>

const int maxParamNameStringLength = 64;
const int maxIPStringLength = 16;
const int maxTimeStampStringLength = 6;

class Packet {

	struct Header {
		char srcIP[maxIPStringLength] = {};
		char destIP[maxIPStringLength] = {};
		int planeID = 0;
		int seq = 0;
	}head;

	struct FlightData {
		char paramName[maxParamNameStringLength] = {};
		char timestamp[maxTimeStampStringLength] = {};
		float fuelTotalQuantity = 0;
	}FD;

	char* TxBuffer = {};

public:
	Packet(char* RxBuffer)
	{
		std::memcpy(&this->head, RxBuffer, sizeof(this->head));
		std::memcpy(&this->FD, RxBuffer + sizeof(this->head), sizeof(this->FD));
	}

	Packet(std::string sourceIP, std::string destinationIP, std::string paramName, int planeID, std::string timestamp, float fuelTotalQuantity)
	{
		std::memcpy(&this->head.srcIP, sourceIP.c_str(), sourceIP.length());
		std::memcpy(&this->head.destIP, destinationIP.c_str(), destinationIP.length());
		this->head.planeID = planeID;
		this->head.seq = 0;
		std::memcpy(&this->FD.timestamp, timestamp.c_str(), timestamp.length());
		std::memcpy(&this->FD.paramName, paramName.c_str(), paramName.length());
		this->FD.fuelTotalQuantity = fuelTotalQuantity;
	}

	char* serialize()
	{
		this->head.seq++;

		this->TxBuffer = (char*)malloc(sizeof(head) + sizeof(FD));

		std::memcpy(this->TxBuffer, &this->head, sizeof(this->head));
		std::memcpy(this->TxBuffer + sizeof(this->head), &this->FD, sizeof(this->FD));

		return this->TxBuffer;
	}

	void incrementSeq()
	{
		this->head.seq++;
	}

	int getSeq() {
		return this->head.seq;
	}

	std::string getTimestamp()
	{
		std::string timestamp = this->FD.timestamp;
		return timestamp;
	}

	float getFuelTotalQuantity()
	{
		return this->FD.fuelTotalQuantity;
	}


};