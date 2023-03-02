#pragma once
#include <string>

//Constants for packet structure
const int maxParamNameStringLength = 64;
const int maxIPStringLength = 16;
const int maxTimeStampStringLength = 6;

/// <summary>
/// Packet Structure for sending FlightData
/// </summary>
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
	/// <summary>
	/// Deserializing Constructor
	/// </summary>
	/// <param name="RxBuffer"></param>
	Packet(char* RxBuffer)
	{
		std::memcpy(&this->head, RxBuffer, sizeof(this->head));
		std::memcpy(&this->FD, RxBuffer + sizeof(this->head), sizeof(this->FD));
	}
	/// <summary>
	/// Packet Creation Constructor
	/// </summary>
	/// <param name="sourceIP"></param>
	/// <param name="destinationIP"></param>
	/// <param name="paramName"></param>
	/// <param name="planeID"></param>
	/// <param name="timestamp"></param>
	/// <param name="fuelTotalQuantity"></param>
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
	/// <summary>
	/// Serialize function
	/// </summary>
	/// <returns>char* buffer</returns>
	char* serialize()
	{
		this->head.seq++;

		this->TxBuffer = (char*)malloc(sizeof(head) + sizeof(FD));

		std::memcpy(this->TxBuffer, &this->head, sizeof(this->head));
		std::memcpy(this->TxBuffer + sizeof(this->head), &this->FD, sizeof(this->FD));

		return this->TxBuffer;
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
	/// Retrieve the Timestamp from the FlightData
	/// </summary>
	/// <returns>std::string timestamp</returns>
	std::string getTimestamp()
	{
		std::string timestamp = this->FD.timestamp;
		return timestamp;
	}
	/// <summary>
	/// Retrieve the FuelTotalQuantity from the FlightData
	/// </summary>
	/// <returns>float FuelTotalQuantity</returns>
	float getFuelTotalQuantity()
	{
		return this->FD.fuelTotalQuantity;
	}


};