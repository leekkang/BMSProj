#include "pch.h"
#include "BMSAdapter.h"

using namespace bms;

/// <summary>
/// make <see cref="BMS::BMSData"/> object by reading the bms file in <paramref name="path"/> and store <see cref="mListRaw"/>
/// </summary>
/// <returns> return true if a <see cref="BMS::BMSData"/> object is correctly build </returns>
bool BMSAdapter::Make(const std::string& path) {
	std::vector<std::string> lines;

	clock_t s = clock();
	if (!Utility::ReadText(path, lines)) {
		LOG("read bms file failed : " + path);
		return false;
	}
	LOG("read text file time(ms) : " << clock() - s)

	BMSDecryptor decryptor(path, lines);

	s = clock();
	if (!decryptor.Build()) {
		LOG("parse bms failed : " + path);
		return false;
	}
	LOG("bms data build time(ms) : " << clock() - s);

	//mListData.push_back(decryptor.CopyBmsData());
	mListData.emplace_back(decryptor.GetBmsData());
	mListFolderPath.emplace_back(Utility::GetDirectory(path) + '/');

	return true;
}

/// <summary>
/// Preview music contained in the <paramref name="index"/> of <paramref name="mListData"/> vector
/// </summary>
void BMSAdapter::Play(int index) {
	if (static_cast<int>(mListData.size()) <= index) {
		LOG("There is no BMSData at the matching index");
		return;
	}

	clock_t s = clock();
	mThread.Play(mListFolderPath[index], mListData[index]);
	LOG("mThread.Play time(ms) : " << clock() - s);
}