#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include "../Shared/configManager.h"
#include "../Shared/Metrics.h"
#include <queue>
#include <filesystem>


void GetSizePromise(std::promise<unsigned int> promise);

/// <summary>
/// This namespace is used to cover file reading for performance requirements IO 001 - 004.
/// the config manager should also be migrated to this namespace in the future.
/// </summary>
namespace fileIO {
	enum statuses { not_started, started, done, error,waiting,reading }; // used to determine the status of a block
	class block {
	private:
		statuses readWrite;// used to block writing to the queue if a read is requested
		void readChunk(char* data); 
		std::promise<int> lengthPromise;
		std::future<int> length = lengthPromise.get_future(); // Promise and future set for size since it needs to wait for a thread to finish to get the line size.
		std::mutex lock; // lock queue when writing.
		statuses status = not_started;
		std::queue<std::string> lines; // list of lines in order
		int size=-1;
	public:
		
		block(char* data);
		int getSize();
		bool hasNext();
		std::string getNext();
		statuses getStatus();


	};


	class fileBuffer {
	private : 
		int length = -1;
		std::vector<block*> chunks;
		int threadCount = 1;
		void splitFile(std::string input);
		statuses status = not_started;
		int currentBlock = 0;
	public:
		fileBuffer(std::string path);
		std::string next();
		bool hasNext();
		int getLineCount();
		
	};

}


/// <summary>
/// initialize the fileBuffer with the path provided uses as many threads as indicated in the config file
/// </summary>
/// <param name="path"></param>
fileIO::fileBuffer::fileBuffer(std::string path) {
	// do everything below in a new thread and return
	FILE* f;//FILE pointer
	int counter = 0;
	fopen_s(&f, path.c_str(), "rb"); // open the file in binary read
	if (f != 0) {
		fseek(f, 0, SEEK_END);// go to end
		long fsize = ftell(f); // get size in bytes by telling the end pointer size
		fseek(f, 0, SEEK_SET); // set pointer back to beginning of file  

		// if greater then 4MB then only malloc 4MB
		char* data = (char*)malloc(fsize + 1);

		//alter fsize to 4MB if bigger 
		fread(data, fsize, 1, f); // read the file into the buffer
		fclose(f); // close

		data[fsize] = 0;// 0 terminate file
		std::string output = data;
		free(data);
		std::function<void()> f = [this, output]() {this->splitFile(output); };
		std::thread thread(f);
		thread.detach();
		std::this_thread::sleep_for(std::chrono::microseconds(10));
		//if greater then 4MB join thread and redo for next chunk
	}
	else {
		this->status = done;
	}
	
}

/// <summary>
/// splits the data passed in into a preset number of threads
/// </summary>
/// <param name="input"></param>
void fileIO::fileBuffer::splitFile(std::string input) {
	if (input.length() <= 0) {
		this->status = done;
		return;
	}
	this->status = started;
	int chunk = input.length()/this->threadCount;
	int offset = 0;
	for (int b = 0; b < this->threadCount-1; b++) { // for all threads except last since it picks up the remainder
		for (int i = (chunk*b)+chunk; i < input.length(); i++) { // chunk *b + offset ---- 8000bytes / 8 threads ---- 1000 + offset ------ offset += char count to next \n
			if (input[i] == '\n') {
				std::string blockString;
				if (offset == 0) {
					blockString = input.substr(offset, i );
				}
				else {
					blockString = input.substr(offset + 1, i - offset-1);
				}
				std::this_thread::sleep_for(std::chrono::microseconds(10));
				block* b = new block((char*)blockString.c_str());
				std::this_thread::sleep_for(std::chrono::microseconds(10));
				this->chunks.push_back(b);
				offset = i;
				i = input.length() + 1;
			}
		}
	}
	std::string blockStringTwo = input.substr(offset, input.length() - offset); // do last thread with its part plus remainder
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	block* d = new block((char*)blockStringTwo.c_str());
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	this->chunks.push_back(d);
	this->status = done;
}

/// <summary>
/// Gets the next line in the file
/// </summary>
/// <returns></returns>
std::string fileIO::fileBuffer::next() {
	if (this->hasNext()) {
		while (this->status != done && this->chunks.size() >= currentBlock) { // if all blocks are added to the vector this can be started, or at least one chunk is added to the vector
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
		if (this->chunks.at(currentBlock)->hasNext()) { // if current block has next return it
			return this->chunks.at(currentBlock)->getNext();
		}
		else { // current block is done and empty
			if (currentBlock != this->chunks.size()) {
				delete(this->chunks.at(currentBlock)); // free memory
				currentBlock++;
				
				return this->next(); // ------------------------------------------------------------------------------------------------------------------------------------------------- Recursive function ----------------------------
			}
			else {
				return std::string();
			}
		}
	}
	return std::string();
}

/// <summary>
/// Checks if there is another available line in the file.
/// </summary>
/// <returns></returns>
bool fileIO::fileBuffer::hasNext() {
	while (this->status != done && this->chunks.size() >= currentBlock) { // if all blocks are added to the vector this can be started, or at least one chunk is added to the vector
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	if (this->chunks.size() > currentBlock) {
		if (this->chunks.at(currentBlock)->hasNext()) { // current block has another
			return true;
		}
		else {
			while (currentBlock < this->chunks.size() - 1) { // go to next block and check if that one has a next and repeat until the end or a next is found.
				delete(this->chunks.at(currentBlock)); // free memory
				currentBlock++;
				if (this->chunks.at(currentBlock)->hasNext()) {
					return true;
				}
			}
			return false;
		}
	}
	return false;
}


/// <summary>
/// Gets the total line count of the file provided in the initializer waits until all blocks have provided a size
/// </summary>
/// <returns></returns>
int fileIO::fileBuffer::getLineCount() {
	int totalSize = 0;
	while (this->status != done) {
	}
		for (int i = 0; i < this->chunks.size(); i++) {
			totalSize+=this->chunks.at(i)->getSize();
		}
	
	return totalSize;
}






/// <summary>
/// gets the size of the file DataFile.txt
/// Note. should run before looking for clients perhaps in a thread
/// </summary>
/// <returns>uiSize (number of lines in the file)</returns>
void GetSizePromise(std::promise<unsigned int> promise)
{
	configuration::configManager configurations("../Shared/config.conf");
	FILE* f;//FILE pointer
	int counter = 0;
	fopen_s(&f, configurations.getConfigChar("dataFile"), "rb"); // open the file in binary read
	
	if (f != 0) {
		fseek(f, 0, SEEK_END);// go to end
		long fsize = ftell(f); // get size in bytes by telling the end pointer size
		fseek(f, 0, SEEK_SET); // set pointer back to beginning of file  

		//if greater then 4 MB only malloc 4MB
		if(fsize< atoi(configurations.getConfigChar("maxBufferFile"))) { // if file size is greater then max bytes allowed on memory
		char* data = (char*)malloc(fsize + 1);
		fread(data, fsize, 1, f); // read the file into the buffer
		fclose(f); // close

		data[fsize] = 0;// 0 terminate file

		for (int i = 0; i < fsize + 1; i++) {

			if (data[i] == '\n') {
				counter++; // add each new line character as a count
			}
		}
		counter++; // incremement 1 since the last line wont have a new line character
		// if it was greater then 4MB free data and do the next part
		promise.set_value(counter);
		free(data); // delete residual data AFTER promise is set so program may continue during cleanup
	}//if
		else {
			int current = 0;
			while (current < fsize) {
				int max = atoi(configurations.getConfigChar("maxBufferFile")); // max bytes that can be read at once
				char* data = (char*)malloc(max + 1); // only allocate max+1 byte each time.
				
				if (current+max > fsize) { // if next block of bytes goes over the total size only read up to the total size
					max = fsize - current;
				}
				current += max; // increment current by the next block size
				fread(data, max, 1, f); // read the file into the buffer
				data[max] = 0;// 0 terminate file

				for (int i = 0; i < max + 1; i++) {

					if (data[i] == '\n') {
						counter++; // add each new line character as a count
					}
				}
				free(data);
			}
			std::cout << counter;
			promise.set_value(counter); // default -1
			fclose(f); // close
		}
	}// if open
	else {
		promise.set_value(-1); // default -1
	}
}

/// <summary>
/// This function is to be used as a thread. The block initiates this thread then moves on to allow loading of the file in the background.
/// </summary>
/// <param name="data"></param>
void fileIO::block::readChunk(char* data)
{
	// move data into string to use substr
	std::string strData;
	strData = data;
	int lineCounter = 0;

	int offset = -1; // used to offset starting point

	for (int i = 0; i < strData.length(); i++) {
		if (strData[i] == '\n') {
			lineCounter++;
			std::string tmp = strData.substr(offset+1, i-offset);
			

			//insert tmp into list
			if (this->readWrite == waiting) { // if waiting for a line add a line then set status to reading so the reader knows its ready
				this->lock.lock();
				this->lines.push(tmp);
				this->lock.unlock();
				this->readWrite = reading;
			}
			else if (this->readWrite == reading) { // if reading dont write until its done reading
				while (this->readWrite == reading) {
					std::this_thread::sleep_for(std::chrono::microseconds(100));
				}
				this->lock.lock();
				this->lines.push(tmp);
				this->lock.unlock();
				this->readWrite = started;
			}
			else if (this->readWrite == started) {// if started and not reading just write new line
				this->lock.lock();
				this->lines.push(tmp);
				this->lock.unlock();
			}
			else { // error
				Logger log;
				log.log(("Block has errored out"), 4, "Errors");
			}


			offset = i;
		}

		if (i == strData.length() - 1) { // last line doesnt have new line
			std::string tmp = strData.substr(offset + 1, i - offset);
			lineCounter++;
			//insert tmp into list
			if (this->readWrite == waiting) { // if waiting for a line add a line then set status to reading so the reader knows its ready
				this->lock.lock();
				this->lines.push(tmp);
				this->lock.unlock();
				this->readWrite = reading;
			}
			else if (this->readWrite == reading) { // if reading dont write until its done reading
				while (this->readWrite == reading) {
					std::this_thread::sleep_for(std::chrono::microseconds(100));
				}
				this->lock.lock();
				this->lines.push(tmp);
				this->lock.unlock();
				this->readWrite = started;
			}
			else if (this->readWrite == started) {// if started and not reading just write new line
				this->lock.lock();
				this->lines.push(tmp);
				this->lock.unlock();
			}
			else { // error
				Logger log;
				log.log(("Block has errored out"), 4, "Errors");
			}
			offset = i;
		}
		
	}
	
	this->status = done;
	this->lengthPromise.set_value(lineCounter);
}

/// <summary>
/// this inits the block object. It takes the input block to be analyzed into lines.
/// </summary>
/// <param name="data"></param>
fileIO::block::block(char* data)
{
	this->status = started;
	this->readWrite = started;
	std::function<void()> f = [this, data]() {this->readChunk(data); };
	std::thread thread(f);
	thread.detach();
	std::this_thread::sleep_for(std::chrono::microseconds(10));
}

/// <summary>
/// This function gets the total number of lines in this chunk of data
/// Note. Returns -1 if getSize is not ready
/// </summary>
/// <returns></returns>
int fileIO::block::getSize()
{
	if (this->size != -1){
		return this->size;
	}
	else if (this->length._Is_ready()) {
		this->size = this->length._Get_value();
		return this->size;
	}
	else {
		return -1;
	}
}

/// <summary>
/// checks if there is another available line in queue. If there isnt it then checks if the block has been finished reading. If the block is done and all lines are pulled from the queue then this returns false. else returns true
/// Since getNext is blocking
/// </summary>
/// <returns></returns>
bool fileIO::block::hasNext()
{
	bool isEmpty = this->lines.empty();
	if (this->status == fileIO::done && isEmpty == true) { // finished reading lines and no more lines.
		return false;
	}
	else {
		return true;
	}
}
/// <summary>
/// WARNING BLOCKING
/// This function gets the next line in the queue. It has priority over writing in the queue - NON-BLOCKING
/// Note. Returns "" if timed out or no lines left
/// </summary>
/// <returns></returns>
std::string fileIO::block::getNext()
{
	if (this->status == fileIO::done && this->lines.empty() == true) { // finished reading lines and no more lines.
		return "";
	}

	std::string result;
	if (this->lines.size() > 0) {
		this->readWrite = fileIO::reading;
		result = this->lines.front(); // read from front
		this->lock.lock();
		this->lines.pop(); // remove from front
		this->lock.unlock();
		this->readWrite = started;
		return result; // return
	}
	else {
		this->readWrite = fileIO::waiting; // inform processing thread this one is waiting for a line so it may add one. -- readChunk should set status to reading once it writes after a waiting status

		while (this->readWrite != reading && this->status != done) { // keep waiting until the writing thread has written to the queue or until 50ms has been reached. -------------------------------------------   IF DEADLOCK CHECK THIS ------------------------------------
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}

		if (this->readWrite == reading || this->readWrite == done) { // can read now since the thread is either done writing or the next one is in the queue
			if (this->lines.size() > 0) {
				this->readWrite = fileIO::reading;
				result = this->lines.front(); // read from front
				this->lock.lock();
				this->lines.pop(); // remove from front
				this->lock.unlock();
				this->readWrite = started;
				return result; // return
			}
		}
	}
	if (this->status != done) {
		this->status = error;
	}
	return "";
}

/// <summary>
/// used to get the status of this block. Not started, Running, Finished, Error
/// </summary>
/// <returns></returns>
fileIO::statuses fileIO::block::getStatus()
{
	return this->status;
}
