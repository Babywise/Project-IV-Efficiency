#include "IO.h"
#include <queue>



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
		promise.set_value(counter);
		free(data); // delete residual data AFTER promise is set so program may continue during cleanup
	}// if open
	else {
		promise.set_value(-1); // default -1
	}
}
/// <summary>
/// This function is to be used as a thread. The block initiates this thread then moves on to allow loading of the file in the background.
/// </summary>
/// <param name="lineLength"></param>
void fileIO::block::readChunk(std::promise<int> lineLength, char* data)
{
}
/// <summary>
/// this inits the block object. It takes the input block to be analyzed into lines.
/// </summary>
/// <param name="data"></param>
fileIO::block::block(char* data)
{
}

/// <summary>
/// WARNING : BLOCKING
/// This function gets the total number of lines in this chunk of data
/// </summary>
/// <returns></returns>
int fileIO::block::getSize()
{
	return 0;
}

/// <summary>
/// WARNING : BLOCKING
/// checks if there is another available line in queue. If there isnt it then checks if the block has been finished reading. If the block is done and all lines are pulled from the queue then this returns false. otherwise it waits
/// for a  new line to be added to the queue
/// </summary>
/// <returns></returns>
bool fileIO::block::hasNext()
{
	return false;
}
/// <summary>
/// This function gets the next line in the queue. It has priority over writing in the queue - NON-BLOCKING
/// Note. Returns null if hasNext returns false.
/// </summary>
/// <returns></returns>
std::string fileIO::block::getNext()
{
	return std::string();
}

/// <summary>
/// used to get the status of this block. Not started, Running, Finished, Error
/// </summary>
/// <returns></returns>
fileIO::statuses fileIO::block::getStatus()
{
	return statuses();
}
