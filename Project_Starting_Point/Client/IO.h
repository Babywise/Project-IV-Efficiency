#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include "../Shared/configManager.h"

void GetSizePromise(std::promise<unsigned int> promise);

/// <summary>
/// This namespace is used to cover file reading for performance requirements IO 001 - 004.
/// the config manager should also be migrated to this namespace in the future.
/// </summary>
namespace fileIO {
	enum statuses { not_started, started, done, error }; // used to determine the status of a block
	class block {
	private:
		std::queue<std::string> lines; // list of lines in order
		void readChunk(std::promise<int> lineLength,char* data); 
		bool preventLock = false; // used to block writing to the queue if a read is requested
		std::promise<int> lengthPromise;
		std::future<int> length = lengthPromise.get_future(); // Promise and future set for size since it needs to wait for a thread to finish to get the line size.
		std::mutex lock; // lock queue when writing.
		statuses status;
	public:
		block(char* data);
		int getSize();
		bool hasNext();
		std::string getNext();
		statuses getStatus();


	};

}