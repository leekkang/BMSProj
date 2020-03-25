#include "pch.h"
#include "BMSAdapter.h"

#include <conio.h>
#include <thread>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <sstream>

namespace fs = std::experimental::filesystem; 
void ShowListFile(std::string path) {
	for (auto &p : fs::directory_iterator(path))  /*get directory */
		std::cout << p.path().filename() << std::endl;   // get file name
}

int main() {
	std::ifstream is("jp.bml");
	std::string str;
	std::string ko;
	std::string jp;
	std::getline(is, str);
	is.close();
	ko = Utility::ToUTF8(str);
	std::cout << "ko UTF : " << ko << '\n' << std::endl;
	std::string locale = setlocale(LC_ALL, "ja_JP"); // Shift_JIS
	jp = Utility::ToUTF8(str);
	std::cout << "jp UTF : " << jp << '\n' << std::endl;
	std::string locale2 = setlocale(LC_ALL, "ko_KR");
	ko = Utility::ToAnsi(ko);
	jp = Utility::ToAnsi(jp);
	std::cout << "ko : " << ko << '\n' << std::endl;
	std::cout << "jp : " << jp << '\n' << std::endl;
	return 0;

	//std::string locale = setlocale(LC_ALL, "ja_JP"); // Shift_JIS
	//std::string locale2 = setlocale(LC_ALL, "ko_KR");

	//bms::FMODWrapper fmod;
	//fmod.Init();
	//for (auto &p : fs::directory_iterator("./")) {
	//	std::cout << p.path().filename() << std::endl;   // get file name
	//	if (fs::is_directory(p.path())) {
	//		for (auto &p2 : fs::directory_iterator(p.path())) {
	//			if (p2.path().extension() == ".ogg") {
	//				fmod.CreateSound(p2.path().u8string(), 1);
	//			}
	//		}
	//	}
	//}

	std::ios::sync_with_stdio(false);
	std::vector<std::string> paths = {u8"./StreamingAssets/XIV - 虚空グラデーション/GRAD_0710_SPA.bml",
									  u8"./StreamingAssets/2011Route - a meadow full of speculation/bwroad10-7a.bml",
									  u8"./StreamingAssets/Glitch Throne - Engine [eFel]/engine_XYZ.bms",
									  u8"./StreamingAssets/Lyrical Signal Revival - Parousia/_parousia_A.bme"

	};
	
	int pathIndex = 0;
	int max = static_cast<int>(paths.size() - 1);
	std::string folderPath = Utility::GetDirectory(paths[pathIndex]);
	//std::string path = "./test.bms";
	bms::BMSAdapter adapter;
	clock_t s = clock();
	for (std::string path : paths) {
		adapter.Make(path);
	}
	std::cout << "make time(ms) : " << std::to_string(clock() - s) << std::endl;

	//std::cout << " bms file list : " << adapter.mListData[0].mTitle << std::endl;
	// TODO : must be implemented in a thread
	adapter.Play(pathIndex);
	std::cout << "play music, bms file path : " << paths[pathIndex] << std::endl;

	// main loop
	while (true) {
		int i = _getch();
		if (i == 27) {
			adapter.TerminateMusic();
			break;
		} else if (i == 224) {
			int newIndex = pathIndex;
			i = _getch();
			if (i == 72) {			// up arrow
				newIndex = pathIndex + 1 > max ? max : pathIndex + 1;
			} else if (i == 80) {	// down arrow
				newIndex = pathIndex - 1 < 0 ? 0 : pathIndex - 1;
			}

			if (newIndex != pathIndex) {
				adapter.TerminateMusic();
				adapter.Play(newIndex);
				pathIndex = newIndex;
			}
		}
		// if multiple threads work
		std::cout << "play music..." << std::endl;
		//std::this_thread::sleep_for(0.1s);
	}
	std::cout << "end" << std::endl;

	return 0;
}

void UnicodeTest() {
	std::string ko("루브잇");
	std::string tko(u8"루브잇");
	std::wstring uko(L"루브잇");
	std::cout << "ansi : " << ko << std::endl;
	std::cout << "ansi to unicode to ansi : " << Utility::WideToAnsi(Utility::AnsiToWide(ko)) << std::endl;
	std::cout << "ansi to utf : " << Utility::ToUTF8(ko) << std::endl;
	std::cout << "ansi to utf to ansi : " << Utility::ToAnsi(Utility::ToUTF8(ko)) << std::endl;
	std::cout << "utf : " << tko << std::endl;
	std::cout << "utf to ansi : " << Utility::ToAnsi(tko) << std::endl;
	std::cout << "utf to ansi to utf : " << Utility::ToUTF8(Utility::ToAnsi(tko)) << std::endl;
	std::cout << "unicode to ansi : " << Utility::WideToAnsi(uko) << std::endl;
	std::cout << "unicode to utf : " << Utility::WideToUTF8(uko) << std::endl;
	std::string jp("XIV - 虚空グラデーション feat. ｂ");
	std::string tjp(u8"XIV - 虚空グラデーション feat. ｂ");
	std::wstring ujp(L"XIV - 虚空グラデーション feat. ｂ");
	std::cout << "ansi : " << jp << std::endl;
	std::cout << "ansi to unicode to ansi : " << Utility::WideToAnsi(Utility::AnsiToWide(jp)) << std::endl;
	std::cout << "ansi to utf : " << Utility::ToUTF8(jp) << std::endl;
	std::cout << "ansi to utf to ansi : " << Utility::ToAnsi(Utility::ToUTF8(jp)) << std::endl;
	std::cout << "utf : " << tjp << std::endl;
	std::cout << "utf to ansi : " << Utility::ToAnsi(tjp) << std::endl;
	std::cout << "utf to ansi to utf : " << Utility::ToUTF8(Utility::ToAnsi(tjp)) << std::endl;
	std::cout << "unicode to ansi : " << Utility::WideToAnsi(ujp) << std::endl;
	std::cout << "unicode to utf : " << Utility::WideToUTF8(ujp) << std::endl;
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