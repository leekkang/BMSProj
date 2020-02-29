#include "pch.h"
#include "BMSHeader.h"

using namespace bms;

/// <summary>
/// store parsed line in appropriate variable of <see cref="BMS::BMSHeader"/> class
/// </summary>
void BMSHeader::StoreHeader(std::string&& line) {
	if (line.rfind("#PLAYER", 0) == 0) {
		mPlayer = std::stoi(line.substr(8));
	} else if (line.rfind("#GENRE", 0) == 0) {
		mGenre = line.substr(7);
	} else if (line.rfind("#TITLE", 0) == 0) {
		mTitle = line.substr(7);
	} else if (line.rfind("#ARTIST", 0) == 0) {
		mArtist = line.substr(8);
	} else if (line.rfind("#BPM", 0) == 0) {
		if (line[4] == ' ') {
			mBpm = std::stoi(line.substr(5));
		} else {	// #BPMXX
			int key = std::stoi(line.substr(4, 2), nullptr, 36);
			mDicBpm[key] = std::stof(line.substr(7));
		}
	} else if (line.rfind("#PLAYLEVEL", 0) == 0) {
		mLevel = std::stoi(line.substr(11));
	} else if (line.rfind("#RANK", 0) == 0) {
		mRank = std::stoi(line.substr(6));
	} else if (line.rfind("#TOTAL", 0) == 0) {
		mTotal = std::stoi(line.substr(7));
	} else if (line.rfind("#STAGEFILE", 0) == 0) {
		mStageFile = line.substr(11);
	} else if (line.rfind("#BANNER", 0) == 0) {
		mBannerFile = line.substr(8);
	} else if (line.rfind("#DIFFICULTY", 0) == 0) {
		mDifficulty = std::stoi(line.substr(12));
	} else if (line.rfind("#WAV", 0) == 0 || line.rfind("#BMP", 0) == 0) {
		int key = std::stoi(line.substr(4, 2), nullptr, 36);
		std::string name = line.substr(7);
		AddFileToDic(line[1] == 'W', key, name);
	} else if (line.rfind("#STOP", 0) == 0) {
		int key = std::stoi(line.substr(5, 2), nullptr, 36);
		mDicStop[key] = std::stof(line.substr(8));
	} else if (line.rfind("#LNTYPE", 0) == 0) {
		int val = std::stoi(line.substr(8));
		if (val == static_cast<int>(LongnoteType::RDM_TYPE_1) || val == static_cast<int>(LongnoteType::MGQ_TYPE)) {
			mLongNoteType = val;
		}
	} else if (line.rfind("#LNOBJ", 0) == 0) {
		int key = std::stoi(line.substr(7), nullptr, 36);
		mLongNoteType = static_cast<int>(LongnoteType::RDM_TYPE_2);
	} else {
		TRACE("This line is not in the correct format : " + line)
		return;
	}
	TRACE("This line is correct format : " + line)
}

/// <summary> add parsed wav or bmp file to dictionary </summary>
/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
void BMSHeader::AddFileToDic(bool bIsWav, int key, const std::string& val) {
	std::string ext = val.substr(val.size() - 3, 3);
	if (bIsWav) {
		mDicWav[key] = std::make_pair(std::move(val), std::move(ext));
		//mDicWav.insert(std::make_pair(key, std::make_pair(val, ext)));
		TRACE("Store wav element : " + std::to_string(key) + ", " + val);
	} else {
		mDicBmp[key] = std::make_pair(std::move(val), std::move(ext));
		TRACE("Store bmp element : " + std::to_string(key) + ", " + val);
	}
}