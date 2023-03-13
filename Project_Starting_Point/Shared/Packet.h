#pragma once
#include <string>

//Constants for packet structure
const int maxParamNameStringLength = 64;
const int maxIPStringLength = 16;
const int maxTimeStampStringLength = 20;

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

	struct FlightResponse {
		float currentFuelConsumption = 0;
		float averageFuelConsumption = 0;
	}FR;

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
		std::memcpy(&this->FR, RxBuffer + sizeof(this->head) + sizeof(this->FD), sizeof(this->FR));
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
	Packet(int planeID)
	{
		this->head.planeID = planeID;
		this->head.seq = 0;
	}
	/// <summary>
	/// Serialize function
	/// </summary>
	/// <returns>char* buffer</returns>
	char* serialize()
	{
		this->head.seq++;

		this->TxBuffer = (char*)malloc(sizeof(head) + sizeof(FD) + sizeof(this->FR));

		std::memcpy(this->TxBuffer, &this->head, sizeof(this->head));
		std::memcpy(this->TxBuffer + sizeof(this->head), &this->FD, sizeof(this->FD));
		std::memcpy(this->TxBuffer + sizeof(this->head) + sizeof(this->FD), &this->FR, sizeof(this->FR));

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
	/// Set the Sequence Number
	/// </summary>
	/// <param name="seq"></param>
	void setSeq(int seq) {
		this->head.seq = seq;
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
	/// <summary>
	/// Retrieve the Paramter Name from the FlightData
	/// </summary>
	/// <returns></returns>
	std::string getParamName()
	{
		std::string paramName = this->FD.paramName;
		return paramName;
	}
	/// <summary>
	/// Set the Current Fuel Value in FlightResponse
	/// </summary>
	/// <param name="currentFuel"></param>
	void setCurrentFuelConsumption(float currentFuel)
	{
		this->FR.currentFuelConsumption = currentFuel;
	}
	float getCurrentFuelConsumption()
	{
		return this->FR.currentFuelConsumption;
	}
	/// <summary>
	/// Set the Average Fuel Value in FlightResponse
	/// </summary>
	/// <param name="avgFuel"></param>
	void setAverageFuelConsumption(float avgFuel)
	{
		this->FR.averageFuelConsumption = avgFuel;
	}
	float getAverageFuelConsumption()
	{
		return this->FR.averageFuelConsumption;
	}
	/// <summary>
	/// Swaps the Source and Destination IPs
	/// </summary>
	void swapIP() {
		char temp[maxIPStringLength] = {};
		strncpy_s(temp, maxIPStringLength, this->head.srcIP, maxIPStringLength);
		strncpy_s(this->head.srcIP, maxIPStringLength, this->head.destIP, maxIPStringLength);
		strncpy_s(this->head.destIP, maxIPStringLength, temp, maxIPStringLength);
	}
	/// <summary>
	/// Retrieve the Plane ID from Head
	/// </summary>
	/// <returns></returns>
	int getPlaneID() {
		return this->head.planeID;
	}
	/// <summary>
	/// Set the timestamp value in FlightData
	/// </summary>
	/// <param name="timestamp"></param>
	void setTimeStamp(std::string timestamp) {
		std::memcpy(this->FD.timestamp, timestamp.c_str(), maxTimeStampStringLength);
	}
	/// <summary>
	/// Retrieves the size of the body contents use for Tx/Rx Buffer
	/// </summary>
	/// <returns>size of packet</returns>
	static int getPacketSize() {
		return sizeof(head) + sizeof(FD) + sizeof(FR);
	}
	void freeBuffer() {
		free(this->TxBuffer);
	}
};