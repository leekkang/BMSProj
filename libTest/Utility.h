#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <fstream>

#define PRINT_LOG 1

#if PRINT_LOG
#define __FILENAME__ (strrchr(__FILE__,'\\')+1)	// change full path to file name
#define TRACE(msg) { std::cout << "[" << __FILENAME__ << ":" << __FUNCTION__ << "():" << __LINE__ << "] : " << msg << std::endl; }
//#define TRACE(msg) { printf("[%s:%s():%d] : %s\n", __FILENAME__, __FUNCTION__, __LINE__, msg); }
//#define TRACE(msg, ...) { printf("[%s:%s():%d] : "msg"\n", __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__); }
#else
#define TRACE(s, ...) ;
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) TypeName(const TypeName&) = delete; \
										   TypeName& operator=(const TypeName&) = delete;

namespace Utility {
	constexpr auto TRIM_SPACE = " \t\n\r";
	//#define TRIM_SPACE " \t\n\r"

	template<typename Out>
	constexpr void Split(const std::string &s, char delim, Out result) {
		std::stringstream ss(s);
		std::string item;
		
		while (getline(ss, item, delim))
			*(result++) = item;
	}
	inline std::vector<std::string> Split(const std::string &s, const char delim) {
		std::vector<std::string> elems;
		Split(s, delim, back_inserter(elems));

		return elems;
	}

	inline std::string trim(std::string& s, const std::string& drop = TRIM_SPACE) {
		std::string r = s.erase(s.find_last_not_of(drop) + 1);
		return r.erase(0, r.find_first_not_of(drop));
	}
	inline std::string Rtrim(std::string& s, const std::string& drop = TRIM_SPACE) {
		return s.erase(s.find_last_not_of(drop) + 1);
	}
	inline std::string Ltrim(std::string& s, const std::string& drop = TRIM_SPACE) {
		return s.erase(0, s.find_first_not_of(drop));
	}

	inline bool HasChar(const std::string& s, int index, char ch) {
		return s.size() > index && s[index] == ch;
	}

	/// <summary>
	/// read file at <c>path</c> and stores it in <paramref name="result"/> vector
	/// </summary>
	inline bool ReadText(const std::string& path, std::vector<std::string>& result) {
		std::ifstream file(path.data());
		if (!file.is_open()) {
			std::cout << "The file does not exist in this path : " << path << std::endl;
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			result.emplace_back(line);
		}

		file.close();
		return true;
	}

	//inline bool ReadBinary(const std::string& path, std::string& result) {}
}