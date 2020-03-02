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
	adapter.Make("./engine.bms");
	TRACE("make time(ms) : " + std::to_string(clock() - s))
	std::cout << "make time(ms) : " << std::to_string(clock() - s) << std::endl;

	//d.Make("path");
	std::cout << "end" << std::endl;

	return 0;
}

void FractionTest() {
	Utility::Fraction a(5, -12);
	Utility::Fraction b(3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -1/24
	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -1/24
	a = Utility::Fraction(5, 12);
	b = Utility::Fraction(-3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	a = Utility::Fraction(5, 12);
	b = Utility::Fraction(3, -8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(3, -8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -19/24
	a = Utility::Fraction(5, -12);
	b = Utility::Fraction(-3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -19/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(3, -8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(-3, 8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	a = Utility::Fraction(5, -12);
	b = Utility::Fraction(-3, -8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -1/24
	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(-3, -8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -1/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(-3, -8);
	a += b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24

	a = Utility::Fraction(5, -12);
	b = Utility::Fraction(3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -19/24
	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -19/24
	a = Utility::Fraction(5, 12);
	b = Utility::Fraction(-3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24
	a = Utility::Fraction(5, 12);
	b = Utility::Fraction(3, -8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24
	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(3, -8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -1/24
	a = Utility::Fraction(5, -12);
	b = Utility::Fraction(-3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -1/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(3, -8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(-3, 8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 19/24
	a = Utility::Fraction(5, -12);
	b = Utility::Fraction(-3, -8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -19/24
	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(-3, -8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -19/24
	a = Utility::Fraction(-5, -12);
	b = Utility::Fraction(-3, -8);
	a -= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // 1/24
	int test = Utility::Pow(10, 1);
	std::cout << test << std::endl;

	a = Utility::Fraction(-5, 12);
	b = Utility::Fraction(-3, -8);
	a *= b; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -5/32
	a *= 2; std::cout << a.GetValue() << ", " << a.mNumerator << ", " << a.mDenominator << std::endl; // -5/16
	std::cout << "end" << std::endl;
}