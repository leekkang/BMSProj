#include "pch.h"
#include "Reader.h"
#include <vector>

constexpr char SEPARATOR {'#'};

int main() {
	std::string str("The sixth sick sheik's sixth sheep's sick.");
	std::string str1("Th.");
	std::string str2("ick.");
	std::string key("sixth");

	bms::BMSData d;
	d.Make("path");
	std::vector<int> v2{1, 2, 3, 4};
	for (int& a : v2) {
		a += 5;
	}
	for (int const& a : v2) {
		std::cout << a << std::endl;
	}

	//TRACE(std::move(str) + str1 + str2 + "dd");
	std::vector<std::string> v = Utility::Split(str, ' ');
	for (std::string s : v) {
		std::cout << s << std::endl;
	}
	std::string st("-3c");
	int aa = std::stoi(st, nullptr, 16);
	std::cout << aa << std::endl;


	std::size_t found = str.rfind(key, 3);
	if (found != std::string::npos)
		str.replace(found, key.length(), "seventh");

	std::cout << str << '\n';

	return 0;
}