#include "pch.h"
#include "BMSAdapter.h"

#include <conio.h>
#include <thread>

int main() {
	//std::ios::sync_with_stdio(false);
	bool bLoading = false;
	int folderIndex = 0;
	int musicIndex = 0;
	short patternIndex = 0;
	bms::BMSAdapter adapter;

	uint16_t folderMax = adapter.GetFolderList().size();
	const std::vector<bms::BMSNode>* musicList = &adapter.GetMusicList(folderIndex);

	auto folderChange = [&](short operand) {
		int oldIndex = folderIndex;
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

	auto musicChange = [&](short operand) {
		int oldIndex = musicIndex;

		musicIndex += operand;
		if (musicIndex < 0) musicIndex = (*musicList).size() - 1;
		else if (musicIndex >= (*musicList).size()) musicIndex = 0;

		if (oldIndex != musicIndex) {
			adapter.TerminateMusic();
			patternIndex = 0;
			adapter.Play((*musicList)[musicIndex].mListData[0]);
		}
	};

	auto patternChange = [&](short operand) {
		short oldIndex = patternIndex;
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
		} else if (i == 91) {		// [
			patternChange(-1);
		} else if (i == 93) {		// ]
			patternChange(1);
		}
		// if multiple threads work
		std::cout << "play music..." << std::endl;
		//std::this_thread::sleep_for(0.1s);
	}
	std::cout << "end" << std::endl;

	return 0;
}