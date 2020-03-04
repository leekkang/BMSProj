#include "pch.h"
#include "BMSAdapter.h"

#include <conio.h>

int main() {
	std::string lpath[] = {"./StreamingAssets/2011Route - a meadow full of speculation/bwroad10-7a.bml",
							"./StreamingAssets/Glitch Throne - Engine [eFel]/engine_XYZ.bms"};

	int pathIndex = 0;
	std::string folderPath = Utility::GetDirectory(lpath[pathIndex]);
	std::string path = lpath[pathIndex];
	//std::string path = "./test.bms";
	bms::BMSAdapter adapter;
	clock_t s = clock();
	adapter.Make(path);
	std::cout << "make time(ms) : " << std::to_string(clock() - s) << std::endl;

	// TODO : must be implemented in a thread
	adapter.Play(0);
	std::cout << "play music, bms file path : " << path << std::endl;

	// main loop
	int i = _getch();
	if (i == 27) { }
	adapter.TerminateMusic();
	//while (adapter.IsPlayingMusic()) {
	//	std::this_thread::sleep_for(1s);
	//	// if multiple threads work, 
	//	printf("music playing...");
	//}
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