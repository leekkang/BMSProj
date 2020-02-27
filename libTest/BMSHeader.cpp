#include "pch.h"
#include "BMSHeader.h"

using namespace BMS;

/// <summary> add parsed wav or bmp file to dictionary </summary>
/// <param name="bIsWav"> flag whether this file is wav or bmp </param>
void BMSHeader::AddFileToDict(bool bIsWav, const std::string& key, const std::string& val) {
	std::string ext = val.substr(val.size() - 3, 3);
	if (bIsWav) {
		mDicWav[key] = std::make_pair(val, ext);
		//mDicWav.insert(std::make_pair(key, std::make_pair(val, ext)));
	} else {
		mDicBmp[key] = std::make_pair(val, ext);
	}
}

///<summary> add parsed stop or bpm command to list </summary>
/// <param name="bIsWav"> flag whether this command is stop or bpm </param>
void BMSHeader::AddCmdToList(bool bIsStop, const std::string& index, const std::string& val) {
	if (bIsStop) {
		mListStop.push_back(std::make_pair(index, val));
	} else {
		mListStop.push_back(std::make_pair(index, val));
	}
}