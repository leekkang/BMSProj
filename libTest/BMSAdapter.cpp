#include "pch.h"
#include "BMSAdapter.h"

using namespace bms;

/// <summary>
/// make <see cref="BMS::BMSData"/> object by reading the bms file in <paramref name="path"/> and store <see cref="mListRaw"/>
/// </summary>
/// <returns> return true if a <see cref="BMS::BMSData"/> object is correctly build </returns>
bool BMSAdapter::Make(std::string path) {
	std::vector<std::string> lines;

	if (!Utility::ReadText(path, lines)) {
		TRACE("read bms file failed : " + path)
		return false;
	}

	BMSDecryptor decryptor(lines);
	BMSData data(path);

	if (!decryptor.Build(data)) {
		TRACE("parse bms failed : " + path)
		return false;
	}

	// sort objects and calculate total beat and measure
	//mData.CalculateBeat();
	data.SetFilePath(path);

	mListData.emplace_back(std::move(data));

	return true;
}
