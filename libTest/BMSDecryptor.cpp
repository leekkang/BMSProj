#include "pch.h"
#include "BMSDecryptor.h"

using namespace bms;

/// <summary>
/// read file and build for fill data in header. no file dictionary is created.
/// Read only information that is displayed on the UI or is helpful when reading information for previewing.
/// </summary>
/// <returns> return true if all line is correctly saved </returns>
bool BMSDecryptor::BuildInfoData(BMSInfoData* data, const wchar_t* path) {
	BMSifstream in(path);
	if (!in.IsOpen()) {
		TRACE("The file does not exist in this path : " + Utility::WideToUTF8(path));
		return false;
	}

	// declare instant variable for parse
	bool hasRandom = false;
	uint16_t wavCnt = 0, measureCnt = 0;
	char prevMeasure[4] = {0,};
	bool b5key = true, bSingle = true;
	int player = 1;
	std::string encodeChecker; encodeChecker.reserve(10240);

	bool isHeader = true;
	std::string line; line.reserve(1024);
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
				encodeChecker.append(pLine + 6);
			} else if (Utility::StartsWith(pLine, "BPM") && *(pLine + 3) == ' ') {
				data->mBpm = data->mMinBpm = data->mMaxBpm = Utility::parseInt(pLine + 4);
			} else if (Utility::StartsWith(pLine, "PLAYER") && length > 7) {
				// single = 1, couple = 2, double = 3
				player = Utility::parseInt(pLine + 7);
			} else if (Utility::StartsWith(pLine, "PLAYLEVEL") && length > 10) {
				data->mLevel = Utility::parseInt(pLine + 10);
			} else if (Utility::StartsWith(pLine, "DIFFICULTY") && length > 11) {
				data->mDifficulty = Utility::parseInt(pLine + 11);
			} else if (Utility::StartsWith(pLine, "GENRE") && length > 6) {
				data->mGenre = std::string(pLine + 6);
				encodeChecker.append(data->mGenre);
			} else if (Utility::StartsWith(pLine, "TITLE") && length > 6) {
				data->mTitle = std::string(pLine + 6);
				encodeChecker.append(data->mTitle);
			} else if (Utility::StartsWith(pLine, "ARTIST") && length > 7) {
				data->mArtist = std::string(pLine + 7);
				encodeChecker.append(data->mArtist);
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
			} else if (!hasRandom && Utility::StartsWith(pLine, "RANDOM")) {
				hasRandom = true;
			}
		}
	}

	data->mHasRandom = hasRandom;
	data->mWavCount = wavCnt;
	data->mMeasureCount = measureCnt + 1;

	// set key type
	data->mKeyType = player == 2 ? (b5key ? KeyType::COUPLE_5 : KeyType::COUPLE_7) :
						 bSingle ? (b5key ? KeyType::SINGLE_5 : KeyType::SINGLE_7) :
								   (b5key ? KeyType::DOUBLE_5 : KeyType::DOUBLE_7);

	// set encoding type
	EncodingType type = in.GetEncodeType();
	if (type == EncodingType::UNKNOWN) {
		type = GetEncodeType(encodeChecker);
	}
	data->mFileType = type;
	data->mFilePath = std::wstring(path);

	return true;
}

/// <summary>
/// build using line in <paramref name="lines"/> list for fill data in header or body
/// </summary>
/// <param name="bPreview"> Read only information for previewing </param>
/// <returns> return true if all line is correctly saved </returns>
bool BMSDecryptor::Build(bool bPreview) {
	// 1. parse raw data line to Object list
	//    At this stage, the header information is completely organized.
	clock_t s = clock();
	// TODO : separate preview and game play
	if (!ParseToPreviewRaw()) {
		LOG("The file does not exist in this path : " + Utility::WideToUTF8(mData.mInfo->mFilePath));
		return false;
	}
	LOG("raw object make time(ms) : " << clock() - s);

	// 2. Create a list that stores the cumulative number of beats per measure 
	//	  with the number of measures found when body parsing.
	s = clock();
	BeatFraction frac;
	for (int i = 0; i < mMeasureCount; ++i) {
		frac += mListBeatInMeasure[i];
		mData.mListCumulativeBeat[i].Set(frac.mNumerator, frac.mDenominator);
	}
	LOG("cumulative beat make time(ms) : " << clock() - s)

	// 3. Create a list that stores the change time point. include time, beats, bpm
	s = clock();
	MakeTimeSegment();
	LOG("time segment make time(ms) : " << clock() - s)

	// 4. Read a list of objects and create a list that stores information such as time and beats of the note.
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
/// parse <paramref name="line"/> for fill header and body data and store parsed line 
/// in appropriate variable and temporary data structure
/// </summary>
bool BMSDecryptor::ParseToPreviewRaw() noexcept {
	BMSifstream in((mData.mInfo->mFilePath).data());
	if (!in.IsOpen()) {
		return false;
	}
	mData.mLongNoteType = LongnoteType::RDM_TYPE_1;

	// initialize
	Reset(mData.mInfo->mMeasureCount);
	const char* extension = mData.mInfo->mSoundExtension.data();
	EncodingType encodingType = mData.mInfo->mFileType;

	// declare instant variable for parse
	bool hasRandom = mData.mInfo->mHasRandom;
	bool ignoreLine = false;
	uint8_t rndDepth = 0, ifDepth = 0;
	std::stack<int> rndValue;

	// lambda function that returns a string modified for a file type
	auto GetUTFString = [](const char* s, EncodingType type) {
		// TODO : unicode file names other than Japanese will be added later.
		if (type == EncodingType::SHIFT_JIS) {
			return Utility::ToUTF8(s, Utility::sJpnLoc);
		} else {
			return std::string(s);
		}
	};

	// lambda function to get beat fraction from string. result = decrypted value * 4 (because decrypted value is measure length)
	auto GetBeatFraction = [](const char* p) {
		int numerator = 0, denominator = 1;
		char c = *p;
		bool bFraction = false;
		while (c) {
			if (c >= '0' && c <= '9') {
				numerator = numerator * 10 + (c - '0');
				if (bFraction) {
					denominator *= 10;
				}
			} else if (c == '.') {
				bFraction = true;
			}
			c = *++p;
		}
		if (bFraction) {
			int gcd = Utility::GCD(numerator, denominator);
			numerator /= gcd;
			denominator /= gcd;
		}
		return BeatFraction(numerator * 4, denominator);
	};

	bool isHeader = true;
	std::string line; line.reserve(1024);
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
		// header phase
		if (isHeader) {
			if (Utility::StartsWith(pLine, "WAV") && length > 6) {
				char* sPoint = &(line[line.size() - 3]);
				strncpy_s(sPoint, 4, extension, 3);
				mData.mListWavName[ParseValue(pLine + 3, 36)] = GetUTFString(pLine + 6, encodingType);
			} else if (Utility::StartsWith(pLine, "BPM") && *(pLine + 3) != ' ' && length > 6) {	// #BPMXX
				mListBpm[ParseValue(pLine + 3, 36)] = static_cast<float>(Utility::parseFloat(pLine + 6));
			} else if (Utility::StartsWith(pLine, "STAGEFILE") && length > 10) {
				mData.mStageFile = std::string(pLine + 10);
			} else if (Utility::StartsWith(pLine, "BANNER") && length > 7) {
				mData.mBannerFile = std::string(pLine + 7);
			} else if (Utility::StartsWith(pLine, "STOP") && length > 7) {
				mListStop[ParseValue(pLine + 4, 36)] = std::abs(Utility::parseInt(pLine + 7));
			} else if (Utility::StartsWith(pLine, "LNTYPE") && length > 7) {
				mData.mLongNoteType = static_cast<LongnoteType>(Utility::parseInt(pLine + 7));
			} else if (Utility::StartsWith(pLine, "LNOBJ") && length > 6) {
				mEndNoteVal = Utility::parseInt(pLine + 6, 0, 36);
				mData.mLongNoteType = LongnoteType::RDM_TYPE_2;
			} else {
				TRACE("This line is discarded : " << line);
			}
			continue;
		}

		// body phase
		if (hasRandom) {
			// process random conditional statement
			if (ignoreLine) {
				if (Utility::StartsWith(pLine, "IF") && length > 3) {
					++ifDepth;
				} else if (Utility::StartsWith(pLine, "ENDIF")) {
					if (ifDepth == rndDepth) {	// if non-ignored line
						ignoreLine = false;
					}
					--ifDepth;
				}
				continue;
			} else if (Utility::StartsWith(pLine, "RANDOM") && length > 7) {
				rndValue.push(Utility::xorshf96() % Utility::parseInt(pLine + 7));
				++rndDepth;
				continue;
			} else if (rndDepth > 0) {
				if (Utility::StartsWith(pLine, "IF") && length > 3) {
					if (rndValue.top() != Utility::parseInt(pLine + 3)) {
						ignoreLine = true;
					}
					++ifDepth;
					continue;
				} else if (Utility::StartsWith(pLine, "ENDIF")) {
					--ifDepth;
					continue;
				} else if (Utility::StartsWith(pLine, "ENDRANDOM")) {
					--rndDepth;
					continue;
				}
			}
		}

		// confirm the line to be discarded
		if (length < 7 || *(pLine + 5) != ':') { // correct line -> 00116:0010F211
			TRACE("This data is not in the correct format : " << line);
			continue;
		} else if (length == 8 && *(pLine + 6) == '0' && *(pLine + 7) == '0') {
			// unnecessary line. -> xxxxx:00
			continue;
		} else if (length > 0xFFFF) {
			TRACE("This data length is too long : " << line);
			continue;
		}
		Channel channel = static_cast<Channel>(ParseValue(pLine + 3, 36));
		if (channel > Channel::NOT_USED && channel < Channel::LANDMINE_START || channel > Channel::LANDMINE_END) {
			LOG("This channel is not implemented (=truncate) : " << line);
			continue;
		}

		// create time signature dictionary for calculate beat
		uint16_t measure = ((*pLine - '0') * 100) + ((*(pLine + 1) - '0') * 10) + (*(pLine + 2) - '0');
		if (channel == Channel::MEASURE_LENGTH) {
			mListBeatInMeasure[measure] = GetBeatFraction(pLine + 6);
			TRACE("Add TimeSignature : " << measure << ", length : " << mListBeatInMeasure[measure].mNumerator << " / " << mListBeatInMeasure[measure].mDenominator);
			continue;
		}
		// discard invalid value except MEASURE_LENGTH channel
		if (length % 2 != 0) {
			TRACE("This data length is not a multiple of 2 : " << line);
			continue;
		}
		// ignore BGA related data
		if (channel == Channel::BGA_BASE || channel == Channel::BGA_POOR || channel == Channel::BGA_LAYER) {
			continue;
		}

		// Separate each beat fragment into objects with information.
		ListPool<Object>& objs = mListObj[measure];
		int item = static_cast<int>(length - 6) / 2;
		for (int i = 0; i < item; ++i) {
			// convert value to base-36, if channel is CHANGE_BPM, convert value to hex
			uint16_t val = ParseValue(pLine + 6 + i * 2, channel == Channel::CHANGE_BPM ? 16 : 36);
			if (val == 0) {
				continue;
			}

			if (channel == Channel::BGM) {
				// add object to vector if this object has own sound file
				if (mData.mListWavName[val] == "") {
					LOG("this object has no sound fild. measure : " << measure << ", fraction : " << i << " / " << item << ", val : " << val);
				} else {
					objs.push(Object(val, measure, channel, i, item));
					++mBgmCount;
				}
			} else if (channel == Channel::CHANGE_BPM || channel == Channel::CHANGE_BPM_BY_KEY || channel == Channel::STOP_BY_KEY) {
				// add object to time segment list
				mListRawTiming.push(Object(val, measure, channel, i, item));
				++mRawTimingCount;
			} else {
				// Override checking is done in another function.
				objs.push(Object(val, measure, channel, i, item));
				++mNoteCount;
			}
		}
	}

	return true;
}

/// <summary>
/// make time segment list contain <see cref="bms::TimeSegment"/> object
/// </summary>
void BMSDecryptor::MakeTimeSegment() {
	long long curTime = 0;
	double curBpm = mData.mInfo->mBpm;
	BeatFraction prevBeat;
	mData.mListTimeSeg.resize(mRawTimingCount);

	// push initial time segment
	mData.mListTimeSeg.push(TimeSegment(0, curBpm, 0, 1));
	TRACE("TimeSegment measure : 0, beat : 0, second : 0, bpm : " + std::to_string(curBpm));

	// sort bpm, time-related object list for use as raw TimeSegment struct list
	mListRawTiming.Sort([](const Object& lhs, const Object& rhs) {
		return lhs.mMeasure != rhs.mMeasure ? lhs.mMeasure < rhs.mMeasure :
			 lhs.mFraction != rhs.mFraction ? lhs.mFraction < rhs.mFraction :
											  lhs.mChannel < rhs.mChannel;
	});

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

		if (obj.mChannel == Channel::STOP_BY_KEY && mListStop[obj.mValue] != 0) {
			// STOP value is the time value of 1/192 of a whole note in 4/4 meter be the unit 1
			// 48 == 1 beat
			mData.mListTimeSeg.push(TimeSegment(curTime, 0, curBeatSum.mNumerator, curBeatSum.mDenominator));
			TRACE("TimeSegment measure : " << curMeasure << ", beat : " << curBeatSum.GetValue() << ", second : " << curTime << ", delta : " << delta << ", bpm : " << 0);
			// value / 48 = beats to stop, time = beat * (60/bpm), 
			// --> stop time = (value * 5) / (bpm * 4)
			TRACE("measure : " << curMeasure << ", obj * value * 5000000ll = " << mListStop[obj.mValue] * 5000000ll << ", curbpm * 4 = " << curBpm * 4 << ", result = " << (mListStop[obj.mValue] * 5000000ll) / (curBpm * 4));
			delta = static_cast<long long>(std::round((mListStop[obj.mValue] * 5000000ll) / (curBpm * 4)));
			curTime += delta;
			mData.mListTimeSeg.push(TimeSegment(curTime, curBpm, curBeatSum.mNumerator, curBeatSum.mDenominator));
		} else if (obj.mChannel == Channel::CHANGE_BPM ||
				  (obj.mChannel == Channel::CHANGE_BPM_BY_KEY && mListBpm[obj.mValue] != 0)) {
			curBpm = obj.mChannel == Channel::CHANGE_BPM ? obj.mValue : mListBpm[obj.mValue];
			mData.mInfo->mMinBpm = std::min(mData.mInfo->mMinBpm, curBpm);
			mData.mInfo->mMaxBpm = std::max(mData.mInfo->mMaxBpm, curBpm);
			mData.mListTimeSeg.push(TimeSegment(curTime, curBpm, curBeatSum.mNumerator, curBeatSum.mDenominator));
		}

		prevBeat = curBeatSum;
		TRACE("TimeSegment measure : " << curMeasure << ", beat : " << curBeatSum.GetValue() << ", second : " << curTime << ", delta : " << delta << ", bpm : " << curBpm);
	}
}

/// <summary>
/// make note list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::Note"/> objects
/// </summary>
void BMSDecryptor::MakeNoteList() {
	mData.mListBgm.resize(mBgmCount);
	mData.mListPlayerNote.resize(mNoteCount);
	// TODO : add logic to remove player notes if they exist on the same channel, same bit

	// true if long note type is RDM type 2
	bool isRDM2 = mData.mLongNoteType == LongnoteType::RDM_TYPE_2;
	// only work of RDM type 2, true if LNOBJ value is one of the indexes of WAV
	bool isExistEndWav = mEndNoteVal != 0 && mData.mListWavName[mEndNoteVal] != "";
	// save each column's last note index. This value is used to determine if this object is a long note.
	int lastIndex[9] = {0};

	auto addLong = [&](int column, const BeatFraction& bf) {
		mData.mListPlayerNote[lastIndex[column]].mType = NoteType::LONG;
		mData.mListPlayerNote[lastIndex[column]].mEndBeat = bf;
		lastIndex[column] = 0;
		mData.mNoteCount--; mData.mLongCount++;
	};
	for (int i = 0; i < mMeasureCount; ++i) {
		// check if this measure has information
		if (mListObj[i].size() == 0) {
			continue;
		}

		ListPool<Object>& objs = mListObj[i];
		// 1) sort all object list by ascending of beats
		objs.Sort([](const Object& lhs, const Object& rhs) ->bool { return lhs.mFraction < rhs.mFraction; });

		// 2) Create two lists: a note list that plays sounds and an object list that plays BGA.
		// TODO : refactor to avoid using GetTimeUsingBeat() functions
		for (unsigned int j = 0; j < objs.size(); ++j) {
			Object& obj = objs[j];
			BeatFraction bf = GetBeats(i, obj.mFraction);
			// BG Note list
			if (obj.mChannel == Channel::BGM) {
				mData.mListBgm.push(Note(obj.mValue, Channel::BGM, GetTimeUsingBeat(bf), bf));
				//TRACE("bgm measure : " << i << ", channel : " << 1 << ", beat : " << bf.GetValue() << ", time : " << GetTimeUsingBeat(bf) << ", value : " << obj.mValue);
				continue;
			}

			// player Note list
			int intCh = static_cast<int>(obj.mChannel);
			int column = intCh % 36 - 1;
			bool bLongNote = obj.mChannel >= Channel::KEY_LONG_START && obj.mChannel < Channel::LANDMINE_START;
			bool bInvisibleNote = obj.mChannel >= Channel::KEY_INVISIBLE_START && !bLongNote;

			// remove invisible note with no sound data
			if (bInvisibleNote) {
				if (mData.mListWavName[obj.mValue] == "") {
					continue;
				}
			}

			// note : RDM type2 shares the long note channel with the normal notes.
			if (isRDM2) {
				// It doesn't make sense that RDM type 2 has a long note channel. invalid value -> remove
				if (bLongNote) {
					continue;
				}
				// check only visible note. invisible note doesn't have long note.
				if (obj.mChannel < Channel::KEY_INVISIBLE_START) {
					// This object is end note of long note -> Do not add to the list.
					if (obj.mValue == mEndNoteVal) {
						// convert note object to bgm object if object value is one of the indexes of WAV (always play sound)
						if (isExistEndWav) {
							mData.mListBgm.push(Note(obj.mValue, Channel::BGM, GetTimeUsingBeat(bf), bf));
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
			mData.mListPlayerNote.push(std::move(pn));
		}
	}
}