#include "pch.h"
#include "BMSParser.h"

constexpr char SEPARATOR {'#'};

int main()
{
	std::string str("The sixth sick sheik's sixth sheep's sick.");
	std::string key("sixth");

	std::vector<std::string> v = Utility::split(str, ' ');
	for (std::string s : v) {
		std::cout << s << std::endl;
	}
	std::string st("3c");
	int aa = std::stoi(st, nullptr, 16);
	std::cout << aa << std::endl;


	std::size_t found = str.rfind(key, 3);
	if (found != std::string::npos)
		str.replace(found, key.length(), "seventh");

	std::cout << str << '\n';

	return 0;
}