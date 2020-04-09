#include "pch.h"
#include "BMSAdapter.h"

#include <conio.h>
#include <thread>

int main() {
	//std::ios::sync_with_stdio(false);

	std::vector<const wchar_t*> paths = {L"./StreamingAssets/XIV - 虚空グラデーション/GRAD_0710_SPA.bml",
										 L"./StreamingAssets/2011Route - a meadow full of speculation/bwroad10-7a.bml",
										 L"./StreamingAssets/Glitch Throne - Engine [eFel]/engine_XYZ.bms",
										 L"./StreamingAssets/Lyrical Signal Revival - Parousia/_parousia_A.bme"

	};

	bool bLoading = false;
	uint16_t folderIndex = 0;
	uint16_t musicIndex = 0;
	uint8_t patternIndex = 0;
	bms::BMSAdapter adapter;

	uint16_t folderMax = adapter.GetFolderList().size();
	const std::vector<bms::BMSNode>* musicList = &adapter.GetMusicList(folderIndex);

	auto folderChange = [&](uint16_t operand) {
		uint16_t oldIndex = folderIndex;
		folderIndex += operand;
		if (folderIndex < 0) folderIndex = folderMax - 1;
		else if (folderIndex >= folderMax) folderIndex = 0;
		
		if (oldIndex != folderIndex) {
			adapter.TerminateMusic();
			bLoading = true;
			std::cout << "folder loading..." << std::endl;
			musicList = &adapter.GetMusicList(folderIndex);
			bLoading = false;

			musicIndex = 0;
			patternIndex = 0;
			adapter.Play((*musicList)[0].mListData[0]);
		}
	};

	auto musicChange = [&](uint16_t operand) {
		uint16_t oldIndex = musicIndex;

		musicIndex += operand;
		if (musicIndex < 0) musicIndex = (*musicList).size() - 1;
		else if (musicIndex >= (*musicList).size()) musicIndex = 0;

		if (oldIndex != musicIndex) {
			adapter.TerminateMusic();
			patternIndex = 0;
			adapter.Play((*musicList)[musicIndex].mListData[0]);
		}
	};

	auto patternChange = [&](uint8_t operand) {
		uint8_t oldIndex = patternIndex;
		const auto& list = (*musicList)[musicIndex].mListData;

		patternIndex += operand;
		if (patternIndex < 0) patternIndex = list.size() - 1;
		else if (patternIndex >= list.size()) patternIndex = 0;

		if (oldIndex != patternIndex) {
			adapter.TerminateMusic();
			adapter.Play(list[patternIndex]);
		}
	};

	adapter.Play((*musicList)[0].mListData[0]);

	// main loop
	while (true) {
		int i = _getch();
		if (bLoading) continue;

		if (i == 27) {
			adapter.TerminateMusic();
			break;
		} else if (i == 224) {
			i = _getch();
			if (i == 72) {			// up arrow
				musicChange(1);
			} else if (i == 80) {	// down arrow
				musicChange(-1);
			} else if (i == 75) {	// left arrow
				folderChange(-1);
			} else if (i == 77) {	// right arrow
				folderChange(1);
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