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
		std::queue<char*> lines; // list of lines in order
		
		int size=-1;

	public:
		int getCurrentSize();
		block(char* data);
		int getSize();
		bool hasNext();
		std::string getNext();
		statuses getStatus();


	};


	class fileBuffer {
	private : 
		statuses paging = statuses::not_started;
		statuses nextStep = statuses::not_started;
		int length = -1;
		std::vector<block*> chunks;
		int threadCount = 24;
		void splitFile(char* input);
		statuses status = not_started;
		int currentBlock = 0;
		std::mutex lock;
		void helper(std::string path);
	public:
		fileBuffer(std::string path);
		std::string next();
		bool hasNext();
		int getLineCount();
		~fileBuffer();
		
	};

}


/// <summary>
/// initialize the fileBuffer with the path provided uses as many threads as indicated in the config file
/// </summary>
/// <param name="path"></param>
fileIO::fileBuffer::fileBuffer(std::string path) {
	std::function t = [this, path]() {this->helper(path); };
	std::thread worker(t);
	worker.detach();
}

void fileIO::fileBuffer::helper(std::string path) {


	FILE* f;//FILE pointer
	int counter = 0;
	fopen_s(&f, path.c_str(), "rb"); // open the file in binary read

	if (f != 0) {
		fseek(f, 0, SEEK_END);// go to end
		long fsize = ftell(f); // get size in bytes by telling the end pointer size
		fseek(f, 0, SEEK_SET); // set pointer back to beginning of file  

		//if greater then 4 MB only malloc 4MB
		if (fsize < 2000000) { // if file size is greater then max bytes allowed on memory
			char* data = (char*)malloc(fsize + 1);
			fread(data, fsize, 1, f); // read the file into the buffer
			fclose(f); // close

			data[fsize] = 0;// 0 terminate file

			std::thread thread(std::move([this,data]() {this->splitFile(data); })); // consumes a lot of memory


			thread.detach();
			std::this_thread::sleep_for(std::chrono::microseconds(10));


		}//if
		else {
			bool flag = false;
			this->paging = started;
			int current = 0;
			while (current < fsize) {
				int max = 2000000; // max bytes that can be read at once
				char* data = (char*)malloc(max + 210); // only allocate max+1 byte each time.

				if (current + max > fsize) { // if next block of bytes goes over the total size only read up to the total size
					max = fsize - current;
					flag = true;
				}
				current += max; // increment current by the next block size
				fread(data, max, 1, f); // read the file into the buffer
				if (!flag) {
					for (int z = 0; z < 200; z++) {
						if (data[max + z - 1] != 13) {
							char* tmp = (char*)malloc(1);

							fread(tmp, 1, 1, f);
							data[max + z] = tmp[0];
							free(tmp);

						}
						else {
							current += z;
							z = 300;
						}

					}
				}
				data[max] = 0;// 0 terminate file


				std::thread thread(std::move([this,data]() {this->splitFile(data); })); // consumes a lot of memory
				thread.detach();
				std::this_thread::sleep_for(std::chrono::microseconds(10));

				while (this->nextStep != done) {

				}
				this->nextStep = started;
				// monitor lines remaining without crashing the system here
				int totalSize = 1001;
				while (totalSize > 0) {
					totalSize = 0;


					for (int counter2 = this->currentBlock; counter2 < this->chunks.size(); counter2++) {

					
							if (this->chunks.at(counter2) != nullptr) {
								int tmp;
								tmp = this->chunks.at(counter2)->getCurrentSize();
								if (tmp < 0) {
									std::cout << "here";
								}
								else {
									totalSize += tmp;
								}
							}
					}
						
						
				}
				std::cout << "here";
				std::this_thread::sleep_for(std::chrono::milliseconds(3));
			}
				
		


		}
			
			fclose(f); // close
			
			this->paging = done;
			this->status = done;
	}// if open
	else {
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
				
				
				if (offset == 0) {
					char* blockString =(char*)malloc(chunk+i+1);
					strncpy_s(blockString, i + 1, input + offset,  i );
					try {
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						block* bl = new block(blockString);
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						this->chunks.push_back(bl);
						offset = i;
						i = strlen(input) + 1;
					}
					catch (std::exception e) {
						std::cout << e.what();
					}
				}
				else {
					char* blockString = (char*)malloc(i -offset+1);
					strncpy_s(blockString, i-offset, input + offset+1, i-offset-1);
					try {
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						block* bl;
						bl = (block*)malloc(sizeof(block));
						bl = new block(blockString);
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						this->lock.lock();
						this->chunks.push_back(bl);
						this->lock.unlock();
						std::cout << "hi";


						offset = i;
						i = strlen(input) + 1;
					}
					catch (std::exception e) {
						std::cout << e.what();
					}
				}
				
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

	this->status = done;
	this->nextStep = done;
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
		while (this->chunks.size() <= currentBlock) {
			if (paging == done && currentBlock == this->chunks.size()) {
				return std::string();
			}
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
	while (!this->nextStep) {

	}
	while (this->status != done && this->chunks.size() >= currentBlock) { // if all blocks are added to the vector this can be started, or at least one chunk is added to the vector
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	if (this->chunks.size() > currentBlock) {
		if (this->chunks.at(currentBlock)->hasNext()) { // current block has another
			return true;
		}
		else {
			while (currentBlock <= this->chunks.size() - 1) { // go to next block and check if that one has a next and repeat until the end or a next is found.
				delete(this->chunks.at(currentBlock)); // free memory
				this->currentBlock++;
				if (this->chunks.size() > currentBlock) {
					if (this->chunks.at(currentBlock)->hasNext()) {
						return true;
					}
				}
			}
			if (this->paging != done) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	if (this->paging != done) {
		return true;
	}
	else {
		return false;
	}
}


/// <summary>
/// Gets the total line count of the file provided in the initializer waits until all blocks have provided a size
/// </summary>
/// <returns></returns>
int fileIO::fileBuffer::getLineCount() {
	int totalSize = 0;
	while (this->chunks.size() != this->threadCount) {

	}
		for (int i = currentBlock; i < this->chunks.size(); i++) {
	
			while (this->chunks.at(i)->getStatus() != done) {

			}
			totalSize+=this->chunks.at(i)->getSize();
		}
	
	return totalSize;
}

inline fileIO::fileBuffer::~fileBuffer()
{
	std::cout << "";
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
				//strData.substr(offset + 1, i - offset)
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
			//= strData.substr(offset + 1, i - offset);
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
	this->readWrite = done;
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
	
	std::thread thread([this,data]() {this->readChunk(data); });
	thread.detach();

	std::this_thread::sleep_for(std::chrono::microseconds(3));

}

int fileIO::block::getCurrentSize() {

	


		
		if (this->lines.size() < 0) {
			std::cout << "here";
		}
	
		return this->lines.size();
	
	
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
		this->size = this->lines.size();
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
	while (isEmpty == true && this->status != done) {
		isEmpty = this->lines.empty();
	}
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
