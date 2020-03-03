#include "pch.h"
#include "BMSAdapter.h"

using namespace bms;

/// <summary>
/// make <see cref="BMS::BMSData"/> object by reading the bms file in <paramref name="path"/> and store <see cref="mListRaw"/>
/// </summary>
/// <returns> return true if a <see cref="BMS::BMSData"/> object is correctly build </returns>
bool BMSAdapter::Make(std::string path) {
	std::vector<std::string> lines;

	clock_t s = clock();
	if (!Utility::ReadText(path, lines)) {
		TRACE("read bms file failed : " + path)
		return false;
	}
	std::cout << "read text file time(ms) : " << clock() - s << std::endl;

	BMSDecryptor decryptor(lines);
	BMSData data(path);

	s = clock();
	if (!decryptor.Build(data)) {
		TRACE("parse bms failed : " + path)
		return false;
	}
	std::cout << "bms data build time(ms) : " << clock() - s << std::endl;

	mListData.emplace_back(std::move(data));

	return true;
}
