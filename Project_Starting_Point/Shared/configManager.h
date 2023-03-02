#pragma once
#include <vector>
#include <string>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <string>
namespace configuration {
	class configManager {
		std::vector<std::string> params;
		std::vector<std::string> paramValues;

	public:
		/// <summary>
		/// inits internal memory with the filename provided.
		/// </summary>
		/// <param name="file"></param>
		configManager(std::string file) {
			std::string strInput;
			unsigned int uiSize = 0;
			std::ifstream ifs(file);

			if (ifs.is_open())
			{
				while (!ifs.eof())
				{
					getline(ifs, strInput);
					params.push_back(strInput.substr(0, strInput.find_first_of('=')-1)); // get param name
					paramValues.push_back(strInput.substr(strInput.find_first_of('=') + 2,(strInput.length()- strInput.find_first_of('=') + 1)));// get associated value
				}
			}
			
		}
		/// <summary>
		/// Gets the configuration from a file in the form of a string
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		std::string getConfig(std::string name) {
			
			for (int i = 0; i < params.size(); i++) { // for all paramaters
				if (strcmp(name.c_str(), params.at(i).c_str()) == 0) { //if param passed in is in list
					return paramValues.at(i);//return the value associated with the param based on index
				}
			}
			return "";
		}
		/// <summary>
		/// returns "" if not found. if the name passed in is found in the file the value for that index will be returned.
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		char* getConfigChar(std::string name) {

			for (int i = 0; i < params.size(); i++) { // for all paramaters
				if (strcmp(name.c_str(), params.at(i).c_str()) == 0) { //if param passed in is in list
					return (char*)paramValues.at(i).c_str();//return the value associated with the param based on index
				}
			}
			return (char*)"";
		}
	};
}