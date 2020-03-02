#include "pch.h"
#include "BMSData.h"

using namespace bms;

BMSData::BMSData(std::string path) : mPlayer(1), mRank(2), mTotal(200), mLongNoteType(LongnoteType::RDM_TYPE_1) {
	std::cout << "BMSData constructor" << std::endl;

	// Do not use it if class contains pointer variables.
	// I don't know why below link throw an error that says an access violation.
	// reference : https://www.sysnet.pe.kr/2/0/4
	//memset(((char *)this) + 4, 0, sizeof(BMSData) - 4);
	// Do not use it if class contains std::vector..!
	//memset(this, 0, sizeof(BMSData));

	mPath = path;
}

/// <summary>
/// build using line in <paramref name="lines"/> list for fill data in header or body
/// </summary>
/// <returns> return true if all line is correctly saved </returns>
bool BMSData::Build(std::vector<std::string>& lines) {
	if (lines.size() == 0) {
		TRACE("This file is empty.");
		return false;
	}

	// 1. parse raw data line to Object list
	//    At this stage, the header information is completely organized.
	bool isHeader = true;
	for (std::string& line : lines) {
		// check incorrect line
		if (line.empty() || line[0] != '#') {
			continue;
		}

		// separate header line and body line
		if (isHeader && line[1] == '0') {	// '0' means that measure 000 is start
			isHeader = false;
		}

		if (isHeader) {
			ParseHeader(Utility::Rtrim(line));
		} else {
			ParseBody(Utility::Rtrim(line));
		}
	}

	// 2. organize bpm, time-related data as TimeSegment struct list


	return true;
}


/// <summary>
/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
/// </summary>
void BMSData::ParseHeader(std::string&& line) noexcept {
	if (line.rfind("#PLAYER", 0) == 0 && line.size() > 8) {
		mPlayer = std::stoi(line.substr(8));
	} else if (line.rfind("#GENRE", 0) == 0 && line.size() > 7) {
		mGenre = line.substr(7);
	} else if (line.rfind("#TITLE", 0) == 0 && line.size() > 7) {
		mTitle = line.substr(7);
	} else if (line.rfind("#ARTIST", 0) == 0 && line.size() > 8) {
		mArtist = line.substr(8);
	} else if (line.rfind("#BPM", 0) == 0) {
		if (line[4] == ' ' && line.size() > 5) {
			mBpm = std::stoi(line.substr(5));
		} else if (line.size() > 7) {	// #BPMXX
			int key = std::stoi(line.substr(4, 2), nullptr, 36);
			mDicBpm[key] = std::stof(line.substr(7));
		}
	} else if (line.rfind("#PLAYLEVEL", 0) == 0 && line.size() > 11) {
		mLevel = std::stoi(line.substr(11));
	} else if (line.rfind("#RANK", 0) == 0 && line.size() > 6) {
		mRank = std::stoi(line.substr(6));
	} else if (line.rfind("#TOTAL", 0) == 0 && line.size() > 7) {
		mTotal = std::stoi(line.substr(7));
	} else if (line.rfind("#STAGEFILE", 0) == 0 && line.size() > 11) {
		mStageFile = line.substr(11);
	} else if (line.rfind("#BANNER", 0) == 0 && line.size() > 8) {
		mBannerFile = line.substr(8);
	} else if (line.rfind("#DIFFICULTY", 0) == 0 && line.size() > 12) {
		mDifficulty = std::stoi(line.substr(12));
	} else if ((line.rfind("#WAV", 0) == 0 || line.rfind("#BMP", 0) == 0) && line.size() > 7) {
		int key = std::stoi(line.substr(4, 2), nullptr, 36);
		std::string name = line.substr(7);
		AddFileToDic(line[1] == 'W', key, name);
	} else if (line.rfind("#STOP", 0) == 0 && line.size() > 8) {
		int key = std::stoi(line.substr(5, 2), nullptr, 36);
		mDicStop[key] = std::stof(line.substr(8));
	} else if (line.rfind("#LNTYPE", 0) == 0 && line.size() > 8) {
		LongnoteType val = static_cast<LongnoteType>(std::stoi(line.substr(8)));
		if (val == LongnoteType::RDM_TYPE_1 || val == LongnoteType::MGQ_TYPE) {
			mLongNoteType = val;
		}
	} else if (line.rfind("#LNOBJ", 0) == 0 && line.size() > 7) {
		int key = std::stoi(line.substr(7), nullptr, 36);
		mLongNoteType = LongnoteType::RDM_TYPE_2;
	} else {
		TRACE("This line is not in the correct format : " + line)
			return;
	}
	TRACE("This line is correct format : " + line)
}

/// <summary>
/// parse <paramref name="line"/> for fill body data and store parsed line in appropriate variable
/// </summary>
void BMSData::ParseBody(std::string&& line) noexcept {
	// example line -> #00116:0010F211
	if (line.size() < 9 || line[6] != ':') {
		TRACE("This data is not in the correct format : " + line)
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
		TRACE("Add TimeSignature : " + std::to_string(measure) + ", length : " + std::to_string(mDicTimeSignature[measure]))
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
			//Object tmp(measure, channel, i, item, val);
			bool isChanged = false;
			for (int j = mListObj.size() - 1; j >= 0 ; --j) {
				Object& obj = mListObj[j];
				if (obj.mMeasure != measure)
					break;

				// very, very rarely happen
				if (obj.mChannel == channel && Utility::CompareDoubleSimple((double)i / obj.mFracIndex, (double)item / obj.mFracDenom)) {
					std::cout << "object change, measure : " + std::to_string(measure) + ", fraction : " +
						std::to_string(i) + " / " + std::to_string(item) + ", val : " + std::to_string(val);
					obj.mValue = val;
					isChanged = true;
					//mListObj.erase(mListObj.begin() + j);
					break;
				}
			}
			if (!isChanged) {
				mListObj.emplace_back(measure, channel, i, item, val);
				//mListObj.push_back(std::move(tmp));
			}
		}
		//TRACE("object create, measure : " + std::to_string(measure) + ", fraction : " +
			  //std::to_string(i) + " / " + std::to_string(item) + ", val : " + std::to_string(val))
	}
	TRACE("vector size : " + std::to_string(mListObj.size()) + ", time(ms) : " + std::to_string(clock() - s))
}


/// <summary> add parsed wav or bmp file to dictionary </summary>
/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
void BMSData::AddFileToDic(bool bIsWav, int key, const std::string& val) {
	std::string ext = val.substr(val.size() - 3, 3);
	if (bIsWav) {
		//mDicWav[key] = std::pair<std::string, std::string>(val, ext);
		//mDicWav.emplace(key, std::make_pair(val, ext));
		mDicWav[key] = std::make_pair(std::move(val), std::move(ext));
		TRACE("Store wav element : " + std::to_string(key) + ", " + val);
	} else {
		mDicBmp[key] = std::make_pair(std::move(val), std::move(ext));
		TRACE("Store bmp element : " + std::to_string(key) + ", " + val);
	}
}