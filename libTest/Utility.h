//#pragma once
#ifndef UTILITY_H
#define UTILITY_H



#include <vector>
#include <string>
#include <sstream>

namespace Utility {
	constexpr auto TRIM_SPACE = " \t\n\r";
	//#define TRIM_SPACE " \t\n\r"

	template<typename Out>
	constexpr void split(const std::string &s, char delim, Out result) {
		std::stringstream ss(s);
		std::string item;

		while (getline(ss, item, delim))
			*(result++) = item;
	}
	inline std::vector<std::string> split(const std::string &s, const char delim) {
		std::vector<std::string> elems;
		split(s, delim, back_inserter(elems));

		return elems;
	}

	inline std::string trim(std::string& s, const std::string& drop = TRIM_SPACE) {
		std::string r = s.erase(s.find_last_not_of(drop) + 1);
		return r.erase(0, r.find_first_not_of(drop));
	}
	inline std::string rtrim(std::string& s, const std::string& drop = TRIM_SPACE) {
		return s.erase(s.find_last_not_of(drop) + 1);
	}
	inline std::string ltrim(std::string& s, const std::string& drop = TRIM_SPACE) {
		return s.erase(0, s.find_first_not_of(drop));
	}
}

#endif // !UTILITY_H