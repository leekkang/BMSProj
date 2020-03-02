#include "pch.h"
#include "BMSAdapter.h"
#include <unordered_map>

constexpr char SEPARATOR {'#'};

class A {
public:
	A() { 
		std::cout << "a constructor" << std::endl; 
		//memset(((char *)this) + 4, 0, sizeof(A) - 4);
		//memset(this, 0, sizeof(A));
	};
	~A() = default;
	void Play() {
		int cc;
		return;
	}
	int a;
	std::string b;
	char c[10];
};
class B {
public:
	B() {
		std::cout << "b constructor" << std::endl;
		//memset(((char *)this) + 4, 0, sizeof(B) - 4);
		memset(this, 0, sizeof(B));
	};
	int a;
	int b;
	char c[10];
};

int main() {
	bms::BMSAdapter adapter;
	clock_t s = clock();
	adapter.Make("./test.bme");
	TRACE("make time(ms) : " + std::to_string(clock() - s))
		std::cout << "make time(ms) : " << std::to_string(clock() - s) << std::endl;
	//d.Make("path");

	return 0;
}