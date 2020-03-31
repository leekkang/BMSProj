#include "pch.h"
#include "BMSDecryptor.h"

using namespace bms;

/// <summary>
/// read file and build for fill data in header. no file dictionary is created.
/// </summary>
/// <returns> return <see cref="bms::BMSPreviewData"/> object if all line is correctly saved </returns>
BMSInfoData BMSDecryptor::BuildInfoData(const wchar_t* path) {
	BMSifstream in(path);
	if (!in.IsOpen()) {
		TRACE("The file does not exist in this path : " + Utility::WideToUTF(path));
		return BMSInfoData();
	}

	BMSInfoData data(path);
	// declare instant variable for parse
	bool isHeader = true;
	uint16_t wavCnt = 0, measureCnt = 0;
	uint8_t rndCnt = 0;
	char prevMeasure[4] = {0,};
	bool b5key = true, bSingle = true;

	std::string line;
	line.reserve(1024);
	while (in.GetLine(line, true)) {
		const char* pLine = line.data();
		// check incorrect line
		if (*pLine != '#') {
			continue;
		}
		++pLine;
		// separate header line and body line
		if (isHeader && *pLine == '0') {	// '0' means that measure 000 is start
			isHeader = false;
		}

		size_t length = line.size() - 1;
		if (isHeader) {
			if (Utility::StartsWith(pLine, "WAV") && length > 6) {
				++wavCnt;
			} else if (Utility::StartsWith(pLine, "BPM") && *(pLine + 3) == ' ') {
				data.mBpm = data.mMinBpm = data.mMaxBpm = Utility::parseInt(pLine + 4);
			} else if (Utility::StartsWith(pLine, "PLAYER") && length > 7) {
				data.mPlayer = Utility::parseInt(pLine + 7);
			} else if (Utility::StartsWith(pLine, "PLAYLEVEL") && length > 10) {
				data.mLevel = Utility::parseInt(pLine + 10);
			} else if (Utility::StartsWith(pLine, "DIFFICULTY") && length > 11) {
				data.mDifficulty = Utility::parseInt(pLine + 11);
			} else if (Utility::StartsWith(pLine, "GENRE") && length > 6) {
				data.mGenre = std::string(pLine + 6);
			} else if (Utility::StartsWith(pLine, "TITLE") && length > 6) {
				data.mTitle = std::string(pLine + 6);
			} else if (Utility::StartsWith(pLine, "ARTIST") && length > 7) {
				data.mArtist = std::string(pLine + 7);
			}
		} else {
			if (length > 7 && *(pLine + 5) == ':') {
				// check measure number
				if (!Utility::StartsWith(prevMeasure, pLine)) {
					uint16_t measure = Utility::parseInt(pLine, 3);
					if (measureCnt < measure) {
						measureCnt = measure;
					}

					memcpy(prevMeasure, pLine, 3);
				}

				// check key type
				if (bSingle && *(pLine + 3) == '2') {
					bSingle = false;
				}
				if (b5key && (*(pLine + 4) == '8' || *(pLine + 4) == '9') &&
							  *(pLine + 3) == (bSingle ? '1' : '2')) {
					b5key = false;
				}
			} else if (Utility::StartsWith(pLine, "RANDOM")) {
				++rndCnt;
			}
		}
	}

	data.mWavCount = wavCnt;
	data.mRndCount = rndCnt;
	data.mMeasureCount = measureCnt;
	data.mKeyType = bSingle ? (b5key ? KeyType::SINGLE_5 : KeyType::SINGLE_7) :
							  (b5key ? KeyType::DOUBLE_5 : KeyType::DOUBLE_7);

	EncodingType type = in.GetEncodeType();
	if (type == EncodingType::UNKNOWN) {
		type = GetEncodeType(data.mGenre + data.mTitle + data.mArtist);
	}
	data.mFileType = type;

	return data;
}

/// <summary>
/// build using line in <paramref name="lines"/> list for fill data in header or body
/// </summary>
/// <param name="bPreview"> Read only information for previewing </param>
/// <returns> return true if all line is correctly saved </returns>
bool BMSDecryptor::Build(bool bPreview) {
	if (mListRaw.size() == 0) {
		TRACE("Bms file is empty.");
		return false;
	}
	std::stack<uint32_t> randomStack;

	// 1. parse raw data line to Object list
	//    At this stage, the header information is completely organized.
	clock_t s = clock();
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

		isHeader ? ParseHeader(Utility::Rtrim(line)) : 
				   ParseBody(Utility::Rtrim(line));
	}
	LOG("raw object make time(ms) : " << clock() - s);

	// 2. Create a list that stores the cumulative number of bits per measure 
	//	  with the number of measures found when body parsing.
	s = clock();
	BeatFraction frac;
	for (int i = 0; i <= mEndMeasure; ++i) {
		frac += GetBeatInMeasure(i);
		mData.mListCumulativeBeat.emplace_back(frac.mNumerator, frac.mDenominator);
	}
	LOG("cumulative beat make time(ms) : " << clock() - s)
	
	// 3. sort bpm, time-related object list for use as raw TimeSegment struct list
	s = clock();
	std::sort(mListRawTiming.begin(), mListRawTiming.end(), [](Object lhs, Object rhs) {
		return lhs.mMeasure != rhs.mMeasure ? lhs.mMeasure < rhs.mMeasure :
			   lhs.mFraction != rhs.mFraction ? lhs.mFraction < rhs.mFraction :
												lhs.mChannel < rhs.mChannel;
	});
	LOG("raw time segment sort time(ms) : " << clock() - s)

	// 4. Create a list that stores the change time point. include time, beats, bpm
	s = clock();
	MakeTimeSegment();
	LOG("time segment make time(ms) : " << clock() - s)

	// 5. Read a list of objects and create a list that stores information such as time and beats of the note.
	s = clock();
	MakeNoteList();
	LOG("note list make time(ms) : " << clock() - s)

	mData.mInfo->mTotalTime = GetTotalPlayTime();

	//LOG("total player note num : " << mData.mListPlayerNote.size())
	LOG("total player normal note num : " << mData.mNoteCount)
	LOG("total player long note num : " << mData.mLongCount)
	/*LOG("total player invisible note num : " << [&](int n) ->int {
		std::vector<PlayerNote>& v = mData.mListPlayerNote;
		for (int i = 0; i < v.size(); ++i) if (v[i].mType == NoteType::INVISIBLE) ++n;
		return n; }(0))
	LOG("total player landmine note num : " << [&](int n) ->int {
		std::vector<PlayerNote>& v = mData.mListPlayerNote;
		for (int i = 0; i < v.size(); ++i) if (v[i].mType == NoteType::LANDMINE) ++n;
		return n; }(0))*/
	return true;
}

/// <summary>
/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
/// </summary>
void BMSDecryptor::ParseHeader(std::string&& line) noexcept {
	//if (line.rfind("#PLAYER", 0) == 0 && line.size() > 8) {
	//	mData.mPlayer = std::stoi(line.substr(8));
	//} else if (line.rfind("#GENRE", 0) == 0 && line.size() > 7) {
	//	mData.mGenre = line.substr(7);
	//} else if (line.rfind("#TITLE", 0) == 0 && line.size() > 7) {
	//	mData.mTitle = line.substr(7);
	//} else if (line.rfind("#ARTIST", 0) == 0 && line.size() > 8) {
	//	mData.mArtist = line.substr(8);
	//} else if (line.rfind("#BPM", 0) == 0) {
	//	if (line[4] == ' ' && line.size() > 5) {
	//		mData.mBpm = mData.mMinBpm = mData.mMaxBpm = std::stoi(line.substr(5));
	//	} else if (line.size() > 7) {	// #BPMXX
	//		int key = std::stoi(line.substr(4, 2), nullptr, 36);
	//		mDicBpm[key] = std::stof(line.substr(7));
	//	}
	//} else if (line.rfind("#PLAYLEVEL", 0) == 0 && line.size() > 11) {
	//	mData.mLevel = std::stoi(line.substr(11));
	//} else if (line.rfind("#RANK", 0) == 0 && line.size() > 6) {
	//	mData.mRank = std::stoi(line.substr(6));
	//} else if (line.rfind("#TOTAL", 0) == 0 && line.size() > 7) {
	//	mData.mTotal = std::stoi(line.substr(7));
	//} else if (line.rfind("#STAGEFILE", 0) == 0 && line.size() > 11) {
	//	mData.mStageFile = line.substr(11);
	//} else if (line.rfind("#BANNER", 0) == 0 && line.size() > 8) {
	//	mData.mBannerFile = line.substr(8);
	//} else if (line.rfind("#DIFFICULTY", 0) == 0 && line.size() > 12) {
	//	mData.mDifficulty = std::stoi(line.substr(12));
	//} else if ((line.rfind("#WAV", 0) == 0 || line.rfind("#BMP", 0) == 0) && line.size() > 7) {
	//	int key = std::stoi(line.substr(4, 2), nullptr, 36);
	//	line[1] == 'W' ? mData.mDicWav[key] = line.substr(7) : 
	//					 mData.mDicBmp[key] = line.substr(7);
	//	//TRACE("Store dictionary element : " + std::to_string(key) + ", " + line.substr(7));
	//} else if (line.rfind("#STOP", 0) == 0 && line.size() > 8) {
	//	int key = std::stoi(line.substr(5, 2), nullptr, 36);
	//	mDicStop[key] = std::stoi(line.substr(8));
	//} else if (line.rfind("#LNTYPE", 0) == 0 && line.size() > 8) {
	//	LongnoteType val = static_cast<LongnoteType>(std::stoi(line.substr(8)));
	//	if (val == LongnoteType::RDM_TYPE_1 || val == LongnoteType::MGQ_TYPE) {
	//		mData.mLongNoteType = val;
	//	}
	//} else if (line.rfind("#LNOBJ", 0) == 0 && line.size() > 7) {
	//	if (mEndNoteVal != 0) {
	//		LOG("LNOBJ value is more than one value. : " << line);
	//	}
	//	mEndNoteVal = std::stoi(line.substr(7), nullptr, 36);
	//	mData.mLongNoteType = LongnoteType::RDM_TYPE_2;
	//} else {
	//	TRACE("This line is not in the correct format : " << line);
	//		return;
	//}
	//TRACE("This line is correct format : " << line)
}

/// <summary>
/// parse <paramref name="line"/> for fill body data and store parsed line in appropriate variable
/// </summary>
void BMSDecryptor::ParseBody(std::string&& line) noexcept {
	// example line -> #00116:0010F211
	if (line.size() < 8 || line[6] != ':') {
		TRACE("This data is not in the correct format : " << line);
			return;
	} else if (line.size() == 9 && line[7] == '0' && line[8] == '0') {
		// unnecessary line. -> #xxxxx:00
		return;
	}

	int measure = std::stoi(line.substr(1, 3));
	Channel channel = static_cast<Channel>(std::stoi(line.substr(4, 2), nullptr, 36));
	if (channel > Channel::NOT_USED && channel < Channel::LANDMINE_START || channel > Channel::LANDMINE_END) {
		LOG("This channel is not implemented (=truncate) : " << line);
		return;
	}

	mEndMeasure = measure > mEndMeasure ? measure : mEndMeasure;

	// create time signature dictionary for calculate beat
	if (channel == Channel::MEASURE_LENGTH) {
		std::string measureLen = line.substr(7);
		std::string::size_type index = measureLen.find('.');
		BeatFraction frac;
		if (index == std::string::npos) {
			mDicMeasureLength.emplace(measure, BeatFraction(std::stoi(measureLen), 1));
		} else {
			mDicMeasureLength.emplace(measure, BeatFraction(std::stoi(measureLen.erase(index, 1)), 
															Utility::Pow(10, static_cast<int>(measureLen.size() - index - 1))));
		}
		TRACE("Add TimeSignature : " << measure << ", length : " << mDicMeasureLength[measure].mNumerator << " / " << mDicMeasureLength[measure].mDenominator);
		return;
	}

	// Separate each beat fragment into objects with information.
	std::vector<Object>& objs = mDicObj[measure];
	int item = static_cast<int>(line.substr(7).size()) / 2;
	for (int i = 0; i < item; ++i) {
		// convert value to base-36, if channel is CHANGE_BPM, convert value to hex
		int val = std::stoi(line.substr(7 + i * 2, 2), nullptr, channel == Channel::CHANGE_BPM ? 16 : 36);
		if (val == 0) {
			continue;
		}

		if (channel == Channel::BGM) {
			// add object to vector if this object has own sound file
			if (mData.mDicWav.count(val) != 0) {
				objs.emplace_back(val, measure, channel, i, item);
			} else {
				LOG("this object has no sound fild. measure : " << measure << ", fraction : " << i << " / " << item << ", val : " << val);
			}
		} else if (channel == Channel::CHANGE_BPM || channel == Channel::CHANGE_BPM_BY_KEY || channel == Channel::STOP_BY_KEY) {
			// add object to time segment list
			mListRawTiming.emplace_back(val, measure, channel, i, item);
		} else {
			// overwrite the value (erase and repush)
			bool isChanged = false;
			for (int j = static_cast<int>(objs.size() - 1); j >= 0; --j) {
				Object& obj = objs[j];
				// overwrite value. very, very rarely happen
				if (obj.mChannel == channel && BeatFraction(i, item) == obj.mFraction) {
					TRACE("object change, measure : " << measure << ", fraction : " << i << " / " << item << ", val : " << val);
					obj.mValue = val;
					isChanged = true;
					break;
				}
			}
			if (!isChanged) {
				objs.emplace_back(val, measure, channel, i, item);
			}
		}
	}
}

/// <summary>
/// make time segment list contain <see cref="bms::TimeSegment"/> object
/// </summary>
void BMSDecryptor::MakeTimeSegment() {
	long long curTime = 0;
	double curBpm = mData.mInfo->mBpm;
	BeatFraction prevBeat;

	// push initial time segment
	mData.mListTimeSeg.emplace_back(0, curBpm, 0, 1);
	TRACE("TimeSegment measure : 0, beat : 0, second : 0, bpm : " + std::to_string(curBpm));

	int count = static_cast<int>(mListRawTiming.size());
	for (int i = 0; i < count; ++i) {
		Object& obj = mListRawTiming[i];
		int curMeasure = obj.mMeasure;

		// Beats are only affected by the length of the measure.
		BeatFraction curBeatSum = GetBeats(curMeasure, obj.mFraction);
		//curTime += (curBeatSum - prevBeat).GetValue() * 60 / curBpm;
		long long delta = ((BeatFraction)(curBeatSum - prevBeat)).GetTime(curBpm);
		curTime += delta;
		//curTime += ((BeatFraction)(curBeatSum - prevBeat)).GetTime(curBpm);

		if (obj.mChannel == Channel::STOP_BY_KEY && mDicStop.count(obj.mValue) != 0) {
			// STOP value is the time value of 1/192 of a whole note in 4/4 meter be the unit 1
			// 48 == 1 beat
			mData.mListTimeSeg.emplace_back(curTime, 0, curBeatSum.mNumerator, curBeatSum.mDenominator);
			TRACE("TimeSegment measure : " << curMeasure << ", beat : " << curBeatSum.GetValue() << ", second : " << curTime << ", delta : " << delta << ", bpm : " << 0);
			// value / 48 = beats to stop, time = beat * (60/bpm), 
			// --> stop time = (value * 5) / (bpm * 4)
			TRACE("measure : " << curMeasure << ", obj * value * 5000000ll = " << mDicStop[obj.mValue] * 5000000ll << ", curbpm * 4 = " << curBpm * 4 << ", result = " << (mDicStop[obj.mValue] * 5000000ll) / (curBpm * 4));
			delta = static_cast<long long>(std::round((mDicStop[obj.mValue] * 5000000ll) / (curBpm * 4)));
			curTime += delta;
			mData.mListTimeSeg.emplace_back(curTime, curBpm, curBeatSum.mNumerator, curBeatSum.mDenominator);
		} else if (obj.mChannel == Channel::CHANGE_BPM ||
				  (obj.mChannel == Channel::CHANGE_BPM_BY_KEY && mDicBpm.count(obj.mValue) != 0)) {
			curBpm = obj.mChannel == Channel::CHANGE_BPM ? obj.mValue : mDicBpm[obj.mValue];
			mData.mInfo->mMinBpm = std::min(mData.mInfo->mMinBpm, curBpm);
			mData.mInfo->mMaxBpm = std::max(mData.mInfo->mMaxBpm, curBpm);
			mData.mListTimeSeg.emplace_back(curTime, curBpm, curBeatSum.mNumerator, curBeatSum.mDenominator);
		}

		prevBeat = curBeatSum;
		TRACE("TimeSegment measure : " << curMeasure << ", beat : " << curBeatSum.GetValue() << ", second : " << curTime << ", delta : " << delta << ", bpm : " << curBpm);
	}
}

/// <summary>
/// make note list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::Note"/> objects
/// </summary>
void BMSDecryptor::MakeNoteList() {
	clock_t s = clock();
	// true if long note type is RDM type 2
	bool isRDM2 = mData.mLongNoteType == LongnoteType::RDM_TYPE_2;
	// only work of RDM type 2, true if LNOBJ value is one of the indexes of WAV
	bool isExistEndWav = mEndNoteVal != 0 && mData.mDicWav.count(mEndNoteVal) != 0;
	// save each column's last note index. This value is used to determine if this object is a long note.
	int lastIndex[9] = {0};

	auto addLong = [&](int column, const BeatFraction& bf) {
		mData.mListPlayerNote[lastIndex[column]].mType = NoteType::LONG;
		mData.mListPlayerNote[lastIndex[column]].mEndBeat = bf;
		lastIndex[column] = 0;
		mData.mNoteCount--; mData.mLongCount++;
	};
	for (int i = 0; i <= mEndMeasure; ++i) {
		// check if this measure has information
		if (mDicObj.count(i) == 0) {
			continue;
		}

		std::vector<Object>& objs = mDicObj[i];
		// 1) sort all object list by ascending of beats
		std::sort(objs.begin(), objs.end(), [](Object lhs, Object rhs) ->bool { return lhs.mFraction < rhs.mFraction; });

		// 2) Create two lists: a note list that plays sounds and an object list that plays BGA.
		// TODO : refactor to avoid using GetTimeUsingBeat() functions
		for (const Object& obj : objs) {
			BeatFraction bf = GetBeats(i, obj.mFraction);
			// BG Note list
			if (obj.mChannel < Channel::KEY_1P_1) {
				std::vector<Note>& container = obj.mChannel == Channel::BGM ? mData.mListBgm : mData.mListBga;
				container.emplace_back(obj.mValue, obj.mChannel, GetTimeUsingBeat(bf), bf);
				continue;
			}

			// player Note list
			int intCh = static_cast<int>(obj.mChannel);
			int column = intCh % 36 - 1;
			bool bLongNote = obj.mChannel >= Channel::KEY_LONG_START && obj.mChannel < Channel::LANDMINE_START;
			bool bInvisibleNote = obj.mChannel >= Channel::KEY_INVISIBLE_START && !bLongNote;
			// note : RDM type2 shares the long note channel with the normal notes.
			if (isRDM2) {
				// It doesn't make sense that RDM type 2 has a long note channel. invalid value -> remove
				if (bLongNote) {
					continue;
				} else if (bInvisibleNote) {
					// remove invisible note with no sound data
					if (mData.mDicWav.count(obj.mValue) == 0) {
						continue;
					}
				}
				// check only visible note. invisible note doesn't have long note.
				if (obj.mChannel < Channel::KEY_INVISIBLE_START) {
					// This object is end note of long note -> Do not add to the list.
					if (obj.mValue == mEndNoteVal) {
						// convert note object to bgm object if object value is one of the indexes of WAV (always play sound)
						if (isExistEndWav) {
							mData.mListBgm.emplace_back(obj.mValue, Channel::BGM, GetTimeUsingBeat(bf), bf);
						}
						addLong(column, bf);
						continue;
					}

					lastIndex[column] = static_cast<int>(mData.mListPlayerNote.size());
				}
			} else {
				// convert long note channel to normal note channel
				if (bLongNote) {
					// This object is end note of long note -> Do not add to the list.
					if (lastIndex[column] != 0) {
						addLong(column, bf);
						continue;
					}

					lastIndex[column] = static_cast<int>(mData.mListPlayerNote.size());
					intCh -= 144;	// 36 * 4
				} else if (bInvisibleNote) {
					// remove invisible note with no sound data
					if (mData.mDicWav.count(obj.mValue) == 0) {
						continue;
					}
				}
			}

			// convert variable note channel to normal note channel
			NoteType type = NoteType::NORMAL;
			if (intCh / 36 >= 13) {
				type = NoteType::LANDMINE;
				intCh -= 432;	// 36 * 12
			} else if (intCh / 36 >= 3) {
				type = NoteType::INVISIBLE;
				intCh -= 72;	// 36 * 2
			} else {
				mData.mNoteCount += 1;
			}

			// make note based on the long note information summarized in the above
			PlayerNote pn(obj.mValue, static_cast<Channel>(intCh), GetTimeUsingBeat(bf), bf, type);
			TRACE("note measure : " << i << ", channel : " << intCh << ", beat : " << bf.GetValue() << ", time : " << pn.mTime << ", value : " << pn.mKey);
			mData.mListPlayerNote.emplace_back(std::move(pn));
		}
		//LOG("note list per measure make time(ms) : " << clock() - s);
	}
}