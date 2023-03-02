#pragma once
#include <string>

const int maxParamNameStringLength = 64;
const int maxIPStringLength = 16;
const int maxTimeStampStringLength = 16;

class Packet {

	struct Header {
		char srcIP[maxIPStringLength] = {};
		char destIP[maxIPStringLength] = {};
		int planeID = 0;
		int seq = 0;
	}head;

	struct FlightData {
		char paramName[maxParamNameStringLength] = {};
		char timestamp[maxIPStringLength] = {};
		float fuelTotalQuantity = 0;
	}FD;

	char* TxBuffer = {};

public:
	Packet(char* RxBuffer)
	{
		std::memcpy(&this->head, RxBuffer, sizeof(this->head));
		std::memcpy(&this->FD, RxBuffer + sizeof(this->head), sizeof(this->FD));
		/*std::memcpy(&this->head.srcIP, RxBuffer + sizeof(this->head), maxIPStringLength);
		std::memcpy(&this->head.destIP, RxBuffer + sizeof(this->head) + maxIPStringLength, maxIPStringLength);
		std::memcpy(&this->FD, RxBuffer + sizeof(this->head) + (maxIPStringLength * 2), sizeof(this->FD));
		std::memcpy(&this->FD.timestamp, RxBuffer + sizeof(this->head) + (maxIPStringLength * 2) + sizeof(this->FD), sizeof(this->FD.timestamp));
		std::memcpy(&this->FD.paramName, RxBuffer + sizeof(this->head) + (maxIPStringLength * 2) + sizeof(this->FD) + sizeof(this->FD.timestamp), sizeof(this->FD.paramName));*/
	}

	Packet(std::string sourceIP, std::string destinationIP, std::string paramName, int planeID, std::string timestamp, float fuelTotalQuantity)
	{
		std::memcpy(&this->head.srcIP, sourceIP.c_str(), sizeof(sourceIP));
		std::memcpy(&this->head.destIP, destinationIP.c_str(), sizeof(destinationIP));
		this->head.planeID = planeID;
		this->head.seq = 0;
		std::memcpy(&this->FD.timestamp, timestamp.c_str(), strlen(timestamp.c_str()));
		std::memcpy(&this->FD.paramName, paramName.c_str(), strlen(paramName.c_str()));
		this->FD.fuelTotalQuantity = fuelTotalQuantity;
	}

	char* serialize()
	{
		this->head.seq++;
		int sizeOfHead = sizeof(head);
		int sizeOfFD = sizeof(FD);
		//int sizeOfHead = sizeof(head) + strlen(this->head.srcIP) + strlen(this->head.destIP);
		//int sizeOfFD = sizeof(FD) + strlen(this->FD.timestamp);

		this->TxBuffer = (char*)malloc(sizeOfHead + sizeOfFD);

		std::memcpy(this->TxBuffer, &this->head, sizeof(this->head));
		std::memcpy(this->TxBuffer + sizeof(this->head), &this->FD, sizeof(this->FD));
		/*std::memcpy(this->TxBuffer + sizeof(this->head), &this->head.srcIP, maxIPStringLength);
		std::memcpy(this->TxBuffer + sizeof(this->head) + maxIPStringLength, &this->head.destIP, maxIPStringLength);

		std::memcpy(this->TxBuffer + sizeof(this->head) + (maxIPStringLength * 2), &this->FD, sizeof(this->FD));
		std::memcpy(this->TxBuffer + sizeof(this->head) + (maxIPStringLength * 2) + sizeof(this->FD), this->FD.timestamp, sizeof(this->FD.timestamp));
		std::memcpy(this->TxBuffer + sizeof(this->head) + (maxIPStringLength * 2) + sizeof(this->FD) + sizeof(this->FD.timestamp), this->FD.paramName, sizeof(this->FD.paramName));*/

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