#include "pch.h"
#include "BMSAdapter.h"

constexpr char SEPARATOR {'#'};

class A {
	A() { std::cout << "BMSData constructor" << std::endl; };
	~A() = default;
	void Play() {
		int cc;
		return;
	}
	int a;
	int b;
};

int main() {
	std::string str("The sixth sick sheik's sixth sheep's sick.");
	std::string str1("Th.");
	std::string str2("ick.");
	std::string key("sixth");

	bms::BMSData d;
	//d.Make("path");
	std::cout << sizeof(std::string) << std::endl;
	std::cout << sizeof(bms::LongnoteType) << std::endl;
	std::cout << sizeof(bms::BMSData) << std::endl;
	std::vector<int> v2{1, 2, 3, 4};
	std::cout << sizeof(char) << std::endl;
	std::cout << sizeof(std::vector<std::string>) << std::endl;
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
	std::cout << sizeof(v) << std::endl;
	std::string st("-3c");
	int aa = std::stoi(st, nullptr, 16);
	std::cout << aa << std::endl;


	std::size_t found = str.rfind(key, 3);
	if (found != std::string::npos)
		str.replace(found, key.length(), "seventh");

	std::cout << str << '\n';

	return 0;
}