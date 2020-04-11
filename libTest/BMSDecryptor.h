#pragma once

#include "BMSData.h"
#include "BMSifstream.h"

#include <algorithm>		// std::min, max, sort
#include <stack>
#include <random>

namespace bms {
	/// <summary>
	/// A data structure containing temporary variables and functions required for `bms data` calculations.
	/// This class is declared as a local variable and will not be saved after the whole process.
	/// </summary>
	class BMSDecryptor {
	public:
		// ----- constructor, operator overloading -----

		BMSDecryptor(BMSData& data) : mData(data) {
			mListStop = new int[MAX_INDEX_LENGTH];
			mListBpm = new float[MAX_INDEX_LENGTH];
		};
		~BMSDecryptor() {
			delete[] mListStop;
			delete[] mListBpm;
		}
		DISALLOW_COPY_AND_ASSIGN(BMSDecryptor)
		BMSDecryptor(BMSDecryptor&& others) noexcept = default;
		BMSDecryptor& operator=(BMSDecryptor&&) noexcept = default;

		// ----- user access function -----

		/// <summary>
		/// read file and build for fill data in header. no file dictionary is created.
		/// Read only information that is displayed on the UI or is helpful when reading information for previewing.
		/// </summary>
		/// <returns> return true if all line is correctly saved </returns>
		bool BuildInfoData(BMSInfoData* data, const wchar_t* path);

		/// <summary>
		/// build using line in <paramref name="lines"/> list for fill data in header or body
		/// </summary>
		/// <param name="bPreview"> Read only information for previewing </param>
		/// <returns> return true if all line is correctly saved </returns>
		bool Build(bool bPreview);
		/// <summary>
		/// parse <paramref name="line"/> for fill header and body data and store parsed line 
		/// in appropriate variable and temporary data structure
		/// </summary>
		bool ParseToPreviewRaw() noexcept;
		/// <summary>
		/// make time segment list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::TimeSegment"/> objects
		/// </summary>
		void MakeTimeSegment();
		/// <summary>
		/// make note list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::Note"/> objects
		/// </summary>
		void MakeNoteList();

		// ----- get, set function -----

		/// <summary>
		/// check what type <paramref name="str"/> is.
		/// check order : UTF-8(include english only) -> EUC_KR(expended to CP949) -> Shift-jis(default)
		/// </summary>
		inline bms::EncodingType GetEncodeType(const std::string& str) {
			// if english only string, return UTF-8
			if (Utility::IsValidUTF8(str.data())) {
				return bms::EncodingType::UTF_8;
			}

			std::wstring kws = Utility::AnsiToWide(str, Utility::sKorLoc);
			std::wstring jws = Utility::AnsiToWide(str, Utility::sJpnLoc);
			// If both locales are converted successfully, it is likely to be Japanese. 
			// because EUC-KR contains Japanese characters, but Shift-jis does not.
			if (kws.size() != 0 && jws.size() != 0) {
				return Utility::WideToAnsi(kws, Utility::sJpnLoc).size() == 0 ?
						bms::EncodingType::SHIFT_JIS : bms::EncodingType::EUC_KR;
			}

			return jws.size() == 0 ? bms::EncodingType::EUC_KR :
									 bms::EncodingType::SHIFT_JIS;
		}

		/// <summary>
		/// Function that returns cumulative number of beats at a specific timing
		/// </summary>
		inline BeatFraction GetBeats(int measure, const BeatFraction& frac) {
			if (measure >= mMeasureCount) {
				return BeatFraction();
			} else if (measure == 0) {
				return frac * mListBeatInMeasure[0];
			}
			return frac * mListBeatInMeasure[measure] + mData.mListCumulativeBeat[measure - 1];
		}

		/// <summary>
		/// Function that returns a time at a specific point using beats.
		/// note : Functions that convert time to beats are not provided because errors can occur during casting.
		/// </summary>
		inline long long GetTimeUsingBeat(const BeatFraction& beat) {
			BeatFraction subtract;
			uint32_t index = static_cast<uint32_t>(mData.mListTimeSeg.size()) - 1;
			for (; index > 0; --index) {
				const TimeSegment& t = mData.mListTimeSeg[index];
				subtract = beat - t.mCurBeat;
				// zero bpm means stop signal. skip it.
				if (subtract >= 0) {
					break;
				}
			}

			// previous saved time + current segment time. if index equals to length, add reversed sign
			const TimeSegment& prev = mData.mListTimeSeg[index];

			return index == 0 ? beat.GetTime(prev.mCurBpm) :
								prev.mCurTime + subtract.GetTime(prev.mCurBpm);
		}

		/// <summary>
		/// Function that returns a total play time
		/// </summary>
		inline long long GetTotalPlayTime() {
			const TimeSegment& seg = mData.mListTimeSeg[mData.mListTimeSeg.size() - 1];
			BeatFraction subtract = mData.mListCumulativeBeat[mMeasureCount - 1] - seg.mCurBeat;

			return seg.mCurTime + subtract.GetTime(seg.mCurBpm);
		}

	private:
		BMSData& mData;

		/// <summary> The number of total measure of current bms data </summary>
		uint16_t mMeasureCount;
		/// <summary> 
		/// if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, 
		/// this value direct wav file key in <see cref="BMSData::mDicWav"/>. 
		/// </summary>
		uint16_t mEndNoteVal;

		uint32_t mRawTimingCount;
		uint32_t mBgmCount;
		uint32_t mNoteCount;
		/// <summary> a list of temporary time data objects </summary>
		ListPool<Object> mListRawTiming;
		/// <summary> a list of data objects (smallest unit), the index is measure number </summary>
		std::vector<ListPool<Object>> mListObj;

		/// <summary> a list of STOP command data, the index is STOP command number </summary>
		int* mListStop;
		/// <summary> a list of BPM command data, the index is BPM command number </summary>
		float* mListBpm;
		/// <summary> a list of beats in one measure, the index is measure number, unit = beat (measure * 4) </summary>
		std::vector<BeatFraction> mListBeatInMeasure;

		/// <summary> reset all member variable </summary>
		/// <param name="measureCount"> Number of Bars. It tells you how many lists you need to create. </param>
		void Reset(uint16_t measureCount) {
			mMeasureCount = measureCount;
			mEndNoteVal = 0;
			mRawTimingCount = 0;
			mBgmCount = 0;
			mNoteCount = 0;
			mListRawTiming.clear();

			memset(mListStop, 0, sizeof(int) * MAX_INDEX_LENGTH);
			memset(mListBpm, 0, sizeof(float) * MAX_INDEX_LENGTH);
			if (mListBeatInMeasure.size() < measureCount) {	// mListMeasureLength and mListObj are the same size list
				mListBeatInMeasure.resize(measureCount);
				mListObj.resize(measureCount);
			}
			for (uint16_t i = 0; i < measureCount; ++i) {
				mListBeatInMeasure[i].Set(4, 1);
				mListObj[i].clear();
			}
		}

		/// <summary> parse function to change the value between "00" and "zz" to integer base <paramref name="radix"/> with no error check </summary>
		inline uint16_t ParseValue(const char* val, const uint16_t radix) noexcept {
			bool bFirstLoop = false;
			char c = *val;
			uint16_t acc = 0;
			do {
				if (*val >= '0' && *val <= '9') {
					c -= '0';
				} else if (*val >= 'A' && *val <= 'Z') {
					c -= 'A' - 10;
				} else if (*val >= 'a' && *val <= 'z') {
					c -= 'a' - 10;
				} else {
					break;
				}
				acc = acc * radix + c;
				c = *++val;
			} while (bFirstLoop = !bFirstLoop);

			return acc;
		}
	};
}