#pragma once
/*
* Author : Danny Smith
* Group : 8 
*  - Nicholas Prince, Islam Ahmed, Raven So, Danny Smith
* Date : March 3, 2023
* Version : 1
* 
* Description : This file buffer is designed to increase speed, efficiency and memory limitations to file reading. This file buffer will read the provided file using the configs from the config file provided
* It will use as many threads as indicated, keep memory to a specific maximum, and read files at blazing speeds in the background allowing lines to be buffered for later use in the background
* 
* Notes. 
*	- New line characters are for windows using \n
*	- Improper configurations can lead to fatel errors
*	- Logging has not yet been implemented
* 
*/
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include "../Shared/configManager.h"
#include "../Shared/Metrics.h"
#include <queue>
#include <filesystem>
#include "../Shared/configManager.h"

/// <summary>
/// gets the size of the file using a promise
/// </summary>
/// <param name="promise"></param>
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
		
		void readChunk(char* data);  // reads the buffer given and inputs lines into the queue
		std::promise<int> lengthPromise; // length of the file
		std::future<int> length = lengthPromise.get_future(); // Promise and future set for size since it needs to wait for a thread to finish to get the line size.
		std::mutex lock; // lock queue when writing.
		statuses status = not_started; // status for block
		std::queue<char*> lines; // list of lines in order
		int size=-1;
	public:
		int getCurrentSize(); // get current number of lines in block
		block(char* data); // start buffering the data into line queue
		int getSize(); // get number of lines in block
		bool hasNext(); // has another line available
		std::string getNext(); // gets the next line in the chunk
		statuses getStatus(); // get current status


	};


	class fileBuffer {
	private : 
		statuses paging = statuses::not_started; // paging the file status for over maz size
		statuses nextStep = statuses::not_started; // if all blocks have been added to queue
		int length = -1; // length of entire file
		std::vector<block*> chunks; // chunks of the file ie each thread analyzing a block
		int threadCount = 24; // count of threads used for each chunk
		void splitFile(char* input); // split the file into smaller chunks
		statuses status = not_started; // status of the buffering
		int currentBlock = 0; // current block to be read from for keeping order
		std::mutex lock; // lock
		void helper(std::string path); // helper function for chunking the file from filebuffer initializer
		configuration::configManager* config;
	public:
		fileBuffer(std::string path,std::string config); // read the file from the path into a buffer
		std::string next(); // get next line in the file
		bool hasNext(); // has another line in the file
		int getLineCount(); // get count of all files
		~fileBuffer(); // destructor
		
	};

}


/// <summary>
/// initialize the fileBuffer with the path provided uses as many threads as indicated in the config file
/// </summary>
/// <param name="path"></param>
fileIO::fileBuffer::fileBuffer(std::string path, std::string config) {
	configuration::configManager* conf= new configuration::configManager(config); //create a config manager from the file provided
	this->config = conf;
	this->threadCount = atoi(this->config->getConfigChar("threadCountIO")); // set threads to use
	if (this->threadCount <= 0) { // ensure not less then 1 thread
		this->threadCount = 1;
	}
	else if (this->threadCount > atoi(this->config->getConfigChar("maxIOThreads")) ){ // ensure not more then max threads
		this->threadCount = atoi(this->config->getConfigChar("maxIOThreads"));
	}
	std::function t = [this, path]() {this->helper(path); }; // begin buffering
	std::thread worker(t);
	worker.detach();
	while (this->chunks.size() ==0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

/// <summary>
/// Helper function that takes a file buffer path and splits the file into the max file size at a time ie 1.5MB 
/// Begins chunking the data into threads to create chunks
/// </summary>
/// <param name="path"></param>
void fileIO::fileBuffer::helper(std::string path) {


	FILE* f;//FILE pointer
	int counter = 0;
	fopen_s(&f, path.c_str(), "rb"); // open the file in binary read

	if (f != 0) {
		fseek(f, 0, SEEK_END);// go to end
		long fsize = ftell(f); // get size in bytes by telling the end pointer size
		fseek(f, 0, SEEK_SET); // set pointer back to beginning of file  

		//if greater then 4 MB only malloc 4MB
		if (fsize < atoi(this->config->getConfigChar("maxBufferFile"))) { // if file size is greater then max bytes allowed on memory
			char* data = (char*)malloc(fsize + 1);
			fread(data, fsize, 1, f); // read the file into the buffer
			fclose(f); // close

			data[fsize] = 0;// 0 terminate file

			std::thread thread(std::move([this,data]() {this->splitFile(data); })); // split the full file into chunks


			thread.detach();
			std::this_thread::sleep_for(std::chrono::microseconds(10));


		}//if
		else { // if file is greater then the max buffer size
			bool flag = false; // if data is at end done keep looking for newline char
			this->paging = started;// start paging status
			int current = 0; // used to keep track of current position in file

			while (current < fsize) {
				int max = atoi(this->config->getConfigChar("maxBufferFile")); // max bytes that can be read at once
				char* data = (char*)malloc(max + atoi(this->config->getConfigChar("searchBytes"))); // only allocate max+1 byte each time.

				if (current + max > fsize) { // if next block of bytes goes over the total size only read up to the total size
					max = fsize - current;
					flag = true;
				}
				current += max; // increment current by the next block size
				fread(data, max, 1, f); // read the file into the buffer
				int searchTo = atoi(this->config->getConfigChar("searchBytes"));
				if (!flag) {
					for (int z = 0; z < searchTo; z++) { // read next bytes of file until new line character is found to not split lines apart and ensure full line readings
						if (data[max + z - 1] != 13) { // if not new line from prevous read then read the next byte
							char* tmp = (char*)malloc(1);

							fread(tmp, 1, 1, f);
							data[max + z] = tmp[0];
							free(tmp);

						}
						else { // if last read was new line escape this loop
							current += z;
							z = searchTo+100;
						}

					}
				}
				data[max] = 0;// 0 terminate file


				std::thread thread(std::move([this,data]() {this->splitFile(data); })); //split the data for this chunk of data into blocks
				thread.detach();
				std::this_thread::sleep_for(std::chrono::microseconds(10));

				while (this->nextStep != done) { // wait for blocks to finish being created

				}
				this->nextStep = started; // set status to nextStep as started



				// monitor lines remaining 
				int totalSize = atoi(this->config->getConfigChar("linesRemainingInBuffer"))+10;
				while (totalSize > atoi(this->config->getConfigChar("linesRemainingInBuffer"))) {
					totalSize = 0;


					for (int counter2 = this->currentBlock; counter2 < this->chunks.size(); counter2++) { // if lines remaining in the buffer is lower then in config start next chunk of data

					
							if (this->chunks.at(counter2) != nullptr) { // if not null pointer ( old tests some blocks got corrupt -- this is a fail safe to prevent crashes
								int tmp;
								tmp = this->chunks.at(counter2)->getCurrentSize();
								if (tmp < 0) {
								}
								else {
									totalSize += tmp;
								}
							}
					}
						
						
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(3));
			}
		}
			fclose(f); // close
			this->paging = done; // all work is done set statuses to done.. Not including nextStep splitFile takes care of that
			this->status = done;
	}// if open
	else { // if file fails to open set all statuses to done
		this->paging = done;
		this->status = done;
		this->nextStep = done;
	}

}

/// <summary>
/// splits the data passed in into a preset number of threads
/// </summary>
/// <param name="input"></param>
void fileIO::fileBuffer::splitFile(char* input) {
	if (strlen(input) <= 0) {
		this->status = done;
		return;
	}
	this->status = started;
	int chunk = strlen(input) /this->threadCount;
	int offset = 0;
	for (int b = 0; b < this->threadCount-1; b++) { // for all threads except last since it picks up the remainder
		for (int i = (chunk*b)+chunk; i < strlen(input); i++) { // chunk *b + offset ---- 8000bytes / 8 threads ---- 1000 + offset ------ offset += char count to next \n
			if (input[i] == '\n') {
				
				if (offset == 0) { // if at beginning of file done account for newline char
					char* blockString =(char*)malloc(chunk+i+1);
					strncpy_s(blockString, i + 1, input + offset,  i ); // copy this chunk of memory
					try {
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						block* bl = new block(blockString); // create a block with this block of data
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						this->chunks.push_back(bl); // add to queue of blocks
						offset = i;
						i = strlen(input) + 1;
					}
					catch (std::exception e) {
						std::cout << e.what();
					}
				}
				else { // not the first chunk of data so account of \n
					char* blockString = (char*)malloc(i -offset+1);
					strncpy_s(blockString, i-offset, input + offset+1, i-offset-1); // copy data
					try {
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						block* bl;
						bl = (block*)malloc(sizeof(block)); // create a block 
						bl = new block(blockString);
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						this->lock.lock();
						this->chunks.push_back(bl); // add block to queue
						this->lock.unlock();


						offset = i;
						i = strlen(input) + 1;
					}
					catch (std::exception e) {
						std::cout << e.what();
					}
				}
				i += atoi(this->config->getConfigChar("minLineLength")); // add minimum line length to skip much closer to the end of line, this reduced checks for \n
			}
		}
	}

	char* blockStringTwo = (char*)malloc(strlen(input) - offset+1);
	if (this->threadCount > 1) {
		strncpy_s(blockStringTwo, strlen(input) - offset + 1, input + offset + 1, strlen(input) - offset);// (input.substr(offset, strlen(input) - offset); // do last thread with its part plus remainder
	}
	else {
		strncpy_s(blockStringTwo, strlen(input) - offset + 1, input + offset, strlen(input) - offset);// (input.substr(offset, strlen(input) - offset); // do last thread with its part plus remainder
	}
	blockStringTwo[strlen(input) - offset] = '\0';
	std::this_thread::sleep_for(std::chrono::microseconds(10));

	free(input);
	
	block* d = new block(blockStringTwo);
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	this->chunks.push_back(d);

	this->status = done; // complete this object
	this->nextStep = done;
}

/// <summary>
/// Gets the next line in the file
/// </summary>
/// <returns></returns>
std::string fileIO::fileBuffer::next() {
	if (this->hasNext()) { // ensure there is a next before returning data
		while (this->status != done && this->chunks.size() >= this->currentBlock) { // if all blocks are added to the vector this can be started, or at least one chunk is added to the vector
			if (this->chunks.size() >= this->currentBlock) { // break from current loop ( this is not working in the while loop ) 
				break;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
		while (this->chunks.size() <= this->currentBlock) { //  wait for there to be another block in queue
			if (paging == done && this->currentBlock == this->chunks.size()) {
				return std::string();
			}
		}
		if (this->chunks.at(this->currentBlock)->hasNext()) { // if current block has next return it
			return this->chunks.at(this->currentBlock)->getNext();
		}
		else { // current block is done and empty
			if (this->currentBlock != this->chunks.size()) {
				delete(this->chunks.at(this->currentBlock)); // free memory
				this->currentBlock++;
				
				return this->next(); // ------------------------------------------------------------------------------------------------------------------------------------------------- Recursive function CAUTION ----------------------------
			}
			else {
				return std::string(); // Return empty string if something went wrong
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
	while (this->nextStep == not_started) {// wait for blocks to be populated to ensure they can be accessed

	}
	while (this->status != done && this->chunks.size() >= this->currentBlock) { // if all blocks are added to the vector this can be started, or at least one chunk is added to the vector
		if (this->chunks.size() >= this->currentBlock) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::microseconds(10)); // reduce cpu utilization
	}
	if (this->chunks.size() > this->currentBlock) {
		if (this->chunks.at(this->currentBlock)->hasNext()) { // current block has another
			return true;
		}
		else {
			while (this->currentBlock <= this->chunks.size() - 1) { // go to next block and check if that one has a next and repeat until the end or a next is found.
				delete(this->chunks.at(this->currentBlock)); // free memory
				this->currentBlock++; // move to next block this ones empty
				if (this->chunks.size() > this->currentBlock) {
					if (this->chunks.at(this->currentBlock)->hasNext()) {
						return true;
					}
				}
			}
			if (this->paging != done) { // paging isnt done and weve made it this far so we know more data will fill up in the queue later
				return true;
			}
			else { // something went wrong theres nothing here
				return false;
			}
		}
	}
	if (this->paging != done) {// paging isnt done and weve made it this far so we know more data will fill up in the queue later
		return true;
	}
	else {// something went wrong theres nothing here
		return false;
	}
}


/// <summary>
/// Gets the total line count of the file provided in the initializer waits until all blocks have provided a size
/// </summary>
/// <returns></returns>
int fileIO::fileBuffer::getLineCount() {
	int totalSize = 0;
	while (this->chunks.size() != this->threadCount) { // ensure all blocks have been started before proceeding since they need to be accessed

	}
		for (int i = this->currentBlock; i < this->chunks.size(); i++) { // for all current blocks to end block get their size and return the sum
	
			while (this->chunks.at(i)->getStatus() != done) {

			}
			totalSize+=this->chunks.at(i)->getSize();
		}
	
	return totalSize;
}

/// <summary>
/// destructor
/// </summary>
inline fileIO::fileBuffer::~fileBuffer()
{

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

	std::string strData(data);
	if (strlen(data) > 1) {
		free(data); // comment this line for tests
	}
		
	int lineCounter = 0;
	int offset = -1; // used to offset starting point

	for (int i = 0; i < strData.length(); i++) {
		if (strData[i] == '\n') {
			lineCounter++;
			
			//insert tmp into list
			if (this->readWrite == waiting) { // if waiting for a line add a line then set status to reading so the reader knows its ready
				this->lock.lock();
				char* temp;
				temp = (char*)malloc(i - offset+2);
				strncpy_s(temp, i - offset + 1, (const char*)strData.c_str() + offset + 1, i - offset);
				this->lines.push(temp);
				this->lock.unlock();
				this->readWrite = reading;
			}
			else if (this->readWrite == reading) { // if reading dont write until its done reading
				while (this->readWrite == reading) {
					std::this_thread::sleep_for(std::chrono::microseconds(100));
				}
				char* temp;
				temp = (char*)malloc(i - offset+1);
				strncpy_s(temp, i - offset + 1, (const char*)strData.c_str() + offset + 1, i - offset);
				this->lock.lock();
				this->lines.push(temp);
				this->lock.unlock();
				this->readWrite = started;
			}
			else if (this->readWrite == started) {// if started and not reading just write new line
				char* temp;
				temp = (char*)malloc(i - offset+2);
				strncpy_s(temp,i-offset+1, (const char*)strData.c_str()+offset+1, i - offset);
				this->lock.lock();
				this->lines.push(temp);
				this->lock.unlock();
			}
			else { // error
				Logger log;
				log.log(("Block has errored out"), 4, "Errors");
			}


			offset = i;
		}

		if (i == strData.length() - 1) { // last line doesnt have new line
			char* tmp;
			tmp = (char*)malloc(i - offset+2);
			strncpy_s(tmp, i - offset + 1, (const char*)strData.c_str() + offset + 1, i - offset);
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
	this->readWrite = done; // complete this chunks reading statuses
	this->lengthPromise.set_value(lineCounter); // set line count promise

}

/// <summary>
/// this inits the block object. It takes the input block to be analyzed into lines.
/// </summary>
/// <param name="data"></param>
fileIO::block::block(char* data)
{

	this->status = started;
	this->readWrite = started; // set statuses to started
	
	std::thread thread([this,data]() {this->readChunk(data); }); // begin reading the data into chunks
	thread.detach();

	std::this_thread::sleep_for(std::chrono::microseconds(3));

}

/// <summary>
/// gets the current number of currently available lines in the buffer
/// </summary>
/// <returns></returns>
int fileIO::block::getCurrentSize() {
		return this->lines.size();
}

/// <summary>
/// This function gets the total number of lines in this chunk of data
/// Note. Returns -1 if getSize is not ready
/// </summary>
/// <returns></returns>
int fileIO::block::getSize()
{
	if (this->size != -1){ // -1 means it hasent been set yet so if it has been set return it

		return this->size;
	}
	else if (this->length._Is_ready()) { // check if the promise is ready since we want all lines to be done before answering total number of lines
		this->size = this->lines.size(); // set this->size so we done need to check the mutex next time
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
	this->lock.lock();
	bool isEmpty = this->lines.empty(); // check if the lines are empty
	this->lock.unlock();
	while (isEmpty == true && this->status != done) { // if its empty and not done then wait
		this->lock.lock();
		isEmpty = this->lines.empty();
		this->lock.unlock();
	}
	//there has been a line added to the queue or its finished doing its work
	if (this->status == fileIO::done && isEmpty == true) { // finished reading lines and no more lines.
		return false;
	}
	else {
		return true;// there is a new line that was added to the queue
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
		free(this->lines.front());
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
				free(this->lines.front());
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
