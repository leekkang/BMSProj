#include "pch.h"
#include "BMSDecryptor.h"

using namespace bms;

/// <summary>
/// build using line in <paramref name="lines"/> list for fill data in header or body
/// </summary>
/// <returns> return true if all line is correctly saved </returns>
bool BMSDecryptor::Build(BMSData& bmsData) {
	if (mListRaw.size() == 0) {
		TRACE("Bms file is empty.");
		return false;
	}

	// 1. parse raw data line to Object list
	//    At this stage, the header information is completely organized.
	bool isHeader = true;
	for (std::string& line : mListRaw) {
		// check incorrect line
		if (line.empty() || line[0] != '#') {
			continue;
		}

		// separate header line and body line
		if (isHeader && line[1] == '0') {	// '0' means that measure 000 is start
			isHeader = false;
		}

		if (isHeader) {
			ParseHeader(Utility::Rtrim(line), bmsData);
		} else {
			ParseBody(Utility::Rtrim(line));
		}
	}

	// 2. Create a list that stores the cumulative number of bits per measure 
	//	  with the number of measures found when body parsing.
	Utility::Fraction frac;
	for (int i = 0; i <= mEndMeasure; ++i) {
		frac += GetBeatInMeasure(i);
		mListCumulativeBeat.emplace_back(frac.mNumerator, frac.mDenominator);
	}

	// 3. organize bpm, time-related data as TimeSegment struct list
	std::sort(mListRawTimeSeg.begin(), mListRawTimeSeg.end(), [](Object lhs, Object rhs) {
		if (lhs.mMeasure == rhs.mMeasure) {
			if (lhs.mChannel == rhs.mChannel) {
				return (rhs.mFraction - lhs.mFraction).GetValue() > 0;
				//return (float)lhs.mFracIndex / lhs.mFracDenom < (float)rhs.mFracIndex / rhs.mFracDenom;
			} else {
				return lhs.mChannel < rhs.mChannel;
			}
		} else {
			return lhs.mMeasure < rhs.mMeasure;
		}
	});

	double curTime = 0;
	double curBpm = bmsData.mBpm;
	Utility::Fraction prevBeat;	// prevBeat + prevFrac
	// push initial time segment
	mListTimeSeg.emplace_back(0, curBpm, 0, 1);
	TRACE("TimeSegment beat : 0, second : 0, bpm : " + std::to_string(curBpm))
	for (int i = 0; i < mListRawTimeSeg.size(); ++i) {
		Object& obj = mListRawTimeSeg[i];
		int curMeasure = obj.mMeasure;

		// Beats are only affected by the length of the measure.
		Utility::Fraction curBeatSum = 
			(curMeasure == 0 ? Utility::Fraction::Zero() :
							   mListCumulativeBeat[curMeasure - 1]) + obj.mFraction * GetBeatInMeasure(curMeasure);
		//double velocity = curBpm / 60;		// equal to beat per second
		curTime += (curBeatSum - prevBeat).GetValue() * 60 / curBpm;

		if (obj.mChannel == Channel::STOP_BY_KEY && mDicStop.count(obj.mValue) != 0) {
			// STOP value is the time value of 1/192 of a whole note in 4/4 meter be the unit 1
			// 48 == 1 beat
			mListTimeSeg.emplace_back(curTime, 0, curBeatSum.mNumerator, curBeatSum.mDenominator);
			// value / 48 = beats to stop, time = beat * (60/bpm), 
			// --> stop time = (value * 5) / (bpm * 4)
			curTime += (obj.mValue * 5) / (curBpm * 4);
			mListTimeSeg.emplace_back(curTime, curBpm, curBeatSum.mNumerator, curBeatSum.mDenominator);
		} else if (obj.mChannel == Channel::CHANGE_BPM || 
				   (obj.mChannel == Channel::CHANGE_BPM_BY_KEY && mDicBpm.count(obj.mValue) != 0)) {
			curBpm = obj.mChannel == Channel::CHANGE_BPM ? obj.mValue : mDicBpm[obj.mValue];
			bmsData.mMinBpm = std::min(bmsData.mMinBpm, curBpm);
			bmsData.mMaxBpm = std::max(bmsData.mMaxBpm, curBpm);
			mListTimeSeg.emplace_back(curTime, curBpm, curBeatSum.mNumerator, curBeatSum.mDenominator);
		}

		prevBeat = curBeatSum;
		TRACE("TimeSegment beat : " + std::to_string(curBeatSum.GetValue()) + ", second : " + std::to_string(curTime) + ", bpm : " + std::to_string(curBpm))
	}

	// 4. Read a list of objects and create a list that stores information such as time and beats of the note.


	return true;
}

Utility::Fraction BMSDecryptor::GetBeatInMeasure(int measure) {
	return mDicTimeSignature.count(measure) == 0 ? Utility::Fraction(4, 1) : mDicTimeSignature[measure] * 4;
}

/// <summary>
/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
/// </summary>
void BMSDecryptor::ParseHeader(std::string&& line, BMSData& bmsData) noexcept {
	if (line.rfind("#PLAYER", 0) == 0 && line.size() > 8) {
		bmsData.mPlayer = std::stoi(line.substr(8));
	} else if (line.rfind("#GENRE", 0) == 0 && line.size() > 7) {
		bmsData.mGenre = line.substr(7);
	} else if (line.rfind("#TITLE", 0) == 0 && line.size() > 7) {
		bmsData.mTitle = line.substr(7);
	} else if (line.rfind("#ARTIST", 0) == 0 && line.size() > 8) {
		bmsData.mArtist = line.substr(8);
	} else if (line.rfind("#BPM", 0) == 0) {
		if (line[4] == ' ' && line.size() > 5) {
			bmsData.mBpm = bmsData.mMinBpm = bmsData.mMaxBpm = std::stoi(line.substr(5));
		} else if (line.size() > 7) {	// #BPMXX
			int key = std::stoi(line.substr(4, 2), nullptr, 36);
			mDicBpm[key] = std::stof(line.substr(7));
		}
	} else if (line.rfind("#PLAYLEVEL", 0) == 0 && line.size() > 11) {
		bmsData.mLevel = std::stoi(line.substr(11));
	} else if (line.rfind("#RANK", 0) == 0 && line.size() > 6) {
		bmsData.mRank = std::stoi(line.substr(6));
	} else if (line.rfind("#TOTAL", 0) == 0 && line.size() > 7) {
		bmsData.mTotal = std::stoi(line.substr(7));
	} else if (line.rfind("#STAGEFILE", 0) == 0 && line.size() > 11) {
		bmsData.mStageFile = line.substr(11);
	} else if (line.rfind("#BANNER", 0) == 0 && line.size() > 8) {
		bmsData.mBannerFile = line.substr(8);
	} else if (line.rfind("#DIFFICULTY", 0) == 0 && line.size() > 12) {
		bmsData.mDifficulty = std::stoi(line.substr(12));
	} else if ((line.rfind("#WAV", 0) == 0 || line.rfind("#BMP", 0) == 0) && line.size() > 7) {
		int key = std::stoi(line.substr(4, 2), nullptr, 36);
		std::string name = line.substr(7);
		std::string ext = name.substr(name.size() - 3, 3);
		if (line[1] == 'W') {
			//mDicWav[key] = std::pair<std::string, std::string>(val, ext);
			//mDicWav.emplace(key, std::make_pair(val, ext));
			mDicWav[key] = std::make_pair(std::move(name), std::move(ext));
		} else {
			mDicBmp[key] = std::make_pair(std::move(name), std::move(ext));
		}
		TRACE("Store dictionary element : " + std::to_string(key) + ", " + name);
	} else if (line.rfind("#STOP", 0) == 0 && line.size() > 8) {
		int key = std::stoi(line.substr(5, 2), nullptr, 36);
		mDicStop[key] = std::stof(line.substr(8));
	} else if (line.rfind("#LNTYPE", 0) == 0 && line.size() > 8) {
		LongnoteType val = static_cast<LongnoteType>(std::stoi(line.substr(8)));
		if (val == LongnoteType::RDM_TYPE_1 || val == LongnoteType::MGQ_TYPE) {
			bmsData.mLongNoteType = val;
		}
	} else if (line.rfind("#LNOBJ", 0) == 0 && line.size() > 7) {
		int key = std::stoi(line.substr(7), nullptr, 36);
		bmsData.mLongNoteType = LongnoteType::RDM_TYPE_2;
	} else {
		TRACE("This line is not in the correct format : " + line)
			return;
	}
	TRACE("This line is correct format : " + line)
}

/// <summary>
/// parse <paramref name="line"/> for fill body data and store parsed line in appropriate variable
/// </summary>
void BMSDecryptor::ParseBody(std::string&& line) noexcept {
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

	mEndMeasure = measure > mEndMeasure ? measure : mEndMeasure;

	// create time signature dictionary for calculate beat
	if (channel == Channel::MEASURE_LENGTH) {
		std::string measureLen = line.substr(7);
		std::string::size_type index = measureLen.find('.');
		Utility::Fraction frac;
		if (index == std::string::npos) {
			mDicTimeSignature.emplace(measure, Utility::Fraction(std::stoi(measureLen), 1));
			//frac = Utility::Fraction(std::stoi(measureLen), 1);
			//mDicTimeSignature[measure] = Utility::Fraction(std::stoi(measureLen), 1);
		} else {
			mDicTimeSignature.emplace(measure, Utility::Fraction(std::stoi(measureLen.erase(index, 1)), 
																 Utility::Pow(10, measureLen.size() - index - 1)));
			//frac = Utility::Fraction(std::stoi(measureLen.erase(index, 1)), Utility::Pow(10, measureLen.size() - index - 1));
			//mDicTimeSignature[measure] = Utility::Fraction(std::stoi(measureLen.erase(index, 1)), Utility::Pow(10, measureLen.size() - index - 1));
		}
		//mDicTimeSignature[measure] = frac;
		TRACE("Add TimeSignature : " + std::to_string(measure) + ", length : " + std::to_string(mDicTimeSignature[measure].GetValue()))
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
		} else if (channel == Channel::CHANGE_BPM || channel == Channel::CHANGE_BPM_BY_KEY || channel == Channel::STOP_BY_KEY) {
			// add object to time segment list
			mListRawTimeSeg.emplace_back(measure, channel, i, item, val);
		} else {
			// overwrite the value (erase and repush)
			//Object tmp(measure, channel, i, item, val);
			bool isChanged = false;
			for (int j = mListObj.size() - 1; j >= 0; --j) {
				Object& obj = mListObj[j];
				if (obj.mMeasure != measure)
					break;

				// overwrite value. very, very rarely happen
				if (obj.mChannel == channel && Utility::CompareDoubleSimple((double)i / obj.mFracIndex, (double)item / obj.mFracDenom)) {
					TRACE("object change, measure : " + std::to_string(measure) + ", fraction : " +
						    std::to_string(i) + " / " + std::to_string(item) + ", val : " + std::to_string(val));
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
void BMSDecryptor::AddFileToDic(bool bIsWav, int key, const std::string& val) {
}