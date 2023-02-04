#pragma once
#include <vector>
#include <string>
#include <cstdarg>
namespace configuration {
	class configManager {
		std::vector<std::string> params;
		std::vector<void*> paramValues;

	public:
		configManager(std::string file) {
			// load params from file into memory
		}
		void* getConfig(std::string name) {
			return (void*)3; // stand in stub
		}
		std::vector<void*> getConfigList(std::string params,...) {
			va_list valist;
			std::vector<void*> list;
			va_start(valist, params); // initialize valist
			for (int i = 0; i < params.size(); i++)
			{
				
				list.push_back((void*)va_arg(valist, std::string).c_str());
			}
			va_end(valist); // clean memory
			list.push_back((void*)3);
			// stand in stub
			return list;
		}
	

	};
}