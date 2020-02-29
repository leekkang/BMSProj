#include "pch.h"
#include "BMSData.h"

using namespace bms;

bool BMSData::Build(std::vector<std::string>& lines) {
	bool isHeader = true;
	for (std::string& line : lines) {
		if (line.empty() || line[0] != '#') {
			continue;
		}

		// separate header line and body line
		if (isHeader && line[1] == '0') {	// '0' means that measure 000 is start
			isHeader = false;
		}

		line = Utility::Rtrim(line);
		if (isHeader) {
			ParseHeader(std::move(line));
		} else {
			ParseBody(std::move(line));
		}
	}

	return true;
}


/// <summary>
/// store parsed line in appropriate variable of <see cref="BMS::BMSHeader"/> class
/// </summary>
void BMSData::ParseHeader(std::string&& line) {
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
		LongnoteType val = static_cast<LongnoteType>(std::stoi(line.substr(8)));
		if (val == LongnoteType::RDM_TYPE_1 || val == LongnoteType::MGQ_TYPE) {
			mLongNoteType = val;
		}
	} else if (line.rfind("#LNOBJ", 0) == 0) {
		int key = std::stoi(line.substr(7), nullptr, 36);
		mLongNoteType = LongnoteType::RDM_TYPE_2;
	} else {
		TRACE("This line is not in the correct format : " + line)
			return;
	}
	TRACE("This line is correct format : " + line)
}

/// <summary>
/// store parsed line in appropriate variable of <see cref="BMS::BMSBody"/> class
/// </summary>
void BMSData::ParseBody(std::string&& line) {
	// example line -> #00116:0010F211
	if (line.size() < 9 || line[6] != ':') {
		TRACE("This data is not in the correct format : " + line);
		return;
	} else if (line.size() == 9 && line[7] == '0' && line[8] == '0') {
		// unnecessary line. -> #xxxxx:00
		return;
	}

	int measure = std::stoi(line.substr(1, 3));
	Channel channel = static_cast<Channel>(std::stoi(line.substr(4, 2), nullptr, 36));
	// create time signature dictionary for calculate beat
	if (channel == Channel::MEASURE_LENGTH) {
		mDicTimeSignature[measure] = std::stof(line.substr(7));
		return;
	}

	clock_t s;
	s = clock();
	int item = line.substr(7).size() / 2;
	for (int i = 0; i < item; ++i) {
		// convert value to base-36, if channel is CHANGE_BPM, convert value to hex
		int val = std::stoi(line.substr(7 + i * 2, 2), nullptr, channel == Channel::CHANGE_BPM ? 16 : 36);
		if (val == 0) {
			continue;
		}

		if (channel == Channel::BGM) {
			mListObj.emplace_back(measure, channel, i, item, val);
		} else {
			// overwrite the value (erase and repush)
			Object tmp(measure, channel, i, item, val);
			for (int j = 0; j < mListObj.size(); ++j) {
				if (tmp.IsSameBeat(mListObj[j])) {
					mListObj.erase(mListObj.begin() + j);
					break;
				}
			}
			mListObj.push_back(std::move(tmp));
		}
		TRACE("object create, \n measure : " + std::to_string(measure) + ", fraction : " +
			  std::to_string(i) + " / " + std::to_string(item) + ", val : " + std::to_string(val))
	}
	TRACE("vector size : " + std::to_string(mListObj.size()) + ", time(ms) : " + std::to_string(clock() - s))
}


/// <summary> add parsed wav or bmp file to dictionary </summary>
/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
void BMSData::AddFileToDic(bool bIsWav, int key, const std::string& val) {
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