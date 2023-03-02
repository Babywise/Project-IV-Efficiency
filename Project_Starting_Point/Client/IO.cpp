#include "IO.h"



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