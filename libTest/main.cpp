﻿#include "pch.h"
#include "BMSAdapter.h"

#include <conio.h>
#include <thread>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <sstream>

#include <Windows.h>

std::vector<std::wstring> locales;
BOOL CALLBACK MyFuncLocaleEx(LPWSTR pStr, DWORD dwFlags, LPARAM lparam) {
	locales.push_back(pStr);
	return TRUE;
}

void PrintAllLocales() {
	EnumSystemLocalesEx(MyFuncLocaleEx, LOCALE_ALL, NULL, NULL);

	for (std::vector<std::wstring>::const_iterator str = locales.begin(); str != locales.end(); ++str)
		std::wcout << *str << std::endl;

	std::wcout << "Total " << locales.size() << " locals found." << std::endl;
}

namespace fs = std::experimental::filesystem; 
void ShowListFile(std::string path) {
	for (auto &p : fs::directory_iterator(path))  /*get directory */
		std::cout << p.path().filename() << std::endl;   // get file name
}

/// <summary>
/// check what type <paramref name="str"/> is.
/// check order : UTF-8(include english only) -> EUC_KR(expended to CP949) -> Shift-jis(default)
/// </summary>
inline bms::EncodingType GetEncodeType(const std::string& str) {
	// if english only string, return UTF-8
	if (Utility::IsValidUTF8(str.data())) {
		return bms::EncodingType::UTF_8;
	}

	std::wstring kws = Utility::AnsiToWide(str, std::locale("Korean"));
	std::wstring jws = Utility::AnsiToWide(str, std::locale("Japanese"));
	// If both locales are converted successfully, it is likely to be Japanese. 
	// because EUC-KR contains Japanese characters, but Shift-jis does not.
	if (kws.size() != 0 && jws.size() != 0) {
		return Utility::WideToAnsi(jws, std::locale("Korean")).size() == 0 ? 
			bms::EncodingType::EUC_KR : bms::EncodingType::SHIFT_JIS;
	}

	return jws.size() == 0 ? bms::EncodingType::EUC_KR : 
							 bms::EncodingType::SHIFT_JIS;
}

int main() {
	//PrintAllLocales();

	std::ifstream is("jp.bml");
	std::string str;
	std::getline(is, str);
	is.close();
	is.open("ko.bml");
	std::string str2;
	std::getline(is, str2);
	is.close();
	is.open("jp2.bml");
	std::string str3;
	std::getline(is, str3);
	is.close();
	is.open("ko2.bml");
	std::string str4;
	std::getline(is, str4);
	is.close();
	
	bms::EncodingType e1 = GetEncodeType(str);
	bms::EncodingType e2 = GetEncodeType(str2);
	bms::EncodingType e3 = GetEncodeType(str3);
	bms::EncodingType e4 = GetEncodeType(str4);
	std::cout << "jp type : " << (e1 == bms::EncodingType::UTF_8 ? "UTF" : 
								 (e1 == bms::EncodingType::EUC_KR ? "kr" : "jp")) << '\n' << std::endl;
	std::cout << "kr type : " << (e2 == bms::EncodingType::UTF_8 ? "UTF" :
								 (e2 == bms::EncodingType::EUC_KR ? "kr" : "jp")) << '\n' << std::endl;
	std::cout << "jp type : " << (e3 == bms::EncodingType::UTF_8 ? "UTF" :
								  (e3 == bms::EncodingType::EUC_KR ? "kr" : "jp")) << '\n' << std::endl;
	std::cout << "kr type : " << (e4 == bms::EncodingType::UTF_8 ? "UTF" :
								 (e4 == bms::EncodingType::EUC_KR ? "kr" : "jp")) << '\n' << std::endl;

	return 0;

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
	std::locale loc("Korean");
	std::cout << "ansi : " << ko << std::endl;
	std::cout << "ansi to unicode to ansi : " << Utility::WideToAnsi(Utility::AnsiToWide(ko, loc), loc) << std::endl;
	std::cout << "ansi to utf : " << Utility::ToUTF8(ko, loc) << std::endl;
	std::cout << "ansi to utf to ansi : " << Utility::ToAnsi(Utility::ToUTF8(ko, loc), loc) << std::endl;
	std::cout << "utf : " << tko << std::endl;
	std::cout << "utf to ansi : " << Utility::ToAnsi(tko, loc) << std::endl;
	std::cout << "utf to ansi to utf : " << Utility::ToUTF8(Utility::ToAnsi(tko, loc), loc) << std::endl;
	std::cout << "unicode to ansi : " << Utility::WideToAnsi(uko, loc) << std::endl;
	std::cout << "unicode to utf : " << Utility::WideToUTF8(uko) << std::endl;
	std::string jp("XIV - 虚空グラデーション feat. ｂ");
	std::string tjp(u8"XIV - 虚空グラデーション feat. ｂ");
	std::wstring ujp(L"XIV - 虚空グラデーション feat. ｂ");
	std::cout << "ansi : " << jp << std::endl;
	std::cout << "ansi to unicode to ansi : " << Utility::WideToAnsi(Utility::AnsiToWide(jp, loc), loc) << std::endl;
	std::cout << "ansi to utf : " << Utility::ToUTF8(jp, loc) << std::endl;
	std::cout << "ansi to utf to ansi : " << Utility::ToAnsi(Utility::ToUTF8(jp, loc), loc) << std::endl;
	std::cout << "utf : " << tjp << std::endl;
	std::cout << "utf to ansi : " << Utility::ToAnsi(tjp, loc) << std::endl;
	std::cout << "utf to ansi to utf : " << Utility::ToUTF8(Utility::ToAnsi(tjp, loc), loc) << std::endl;
	std::cout << "unicode to ansi : " << Utility::WideToAnsi(ujp, loc) << std::endl;
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