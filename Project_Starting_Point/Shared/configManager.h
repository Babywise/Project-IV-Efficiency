#pragma once
#include <vector>
#include <string>
#include <cstdarg>
#include <iostream>
#include <fstream>
namespace configuration {
	class configManager {
		std::vector<std::string> params;
		std::vector<std::string> paramValues;

	public:
		configManager(std::string file) {
			string strInput;
			unsigned int uiSize = 0;
			ifstream ifs(file);
			if (ifs.is_open())
			{
				while (!ifs.eof())
				{
					getline(ifs, strInput);
					params.push_back(strInput.substr(0, strInput.find_first_of('=')-1)); // get param
					paramValues.push_back(strInput.substr(strInput.find_first_of('=') + 2,(strInput.length()- strInput.find_first_of('=') + 1)));// get assosciated value
				}
			}
			
		}
		string getConfig(std::string name) {
			
			for (int i = 0; i < params.size(); i++) {
				if (strcmp(name.c_str(), params.at(i).c_str()) == 0) { //if param passed in is in list
					return paramValues.at(i);
				}
			}
			return "";
		}
	};
}