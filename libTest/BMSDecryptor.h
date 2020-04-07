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
		bool ParseToRaw(bool bPreview) noexcept;
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

			std::wstring kws = Utility::AnsiToWide(str, std::locale("Korean"));
			std::wstring jws = Utility::AnsiToWide(str, std::locale("Japanese"));
			// If both locales are converted successfully, it is likely to be Japanese. 
			// because EUC-KR contains Japanese characters, but Shift-jis does not.
			if (kws.size() != 0 && jws.size() != 0) {
				return Utility::WideToAnsi(kws, std::locale("Japanese")).size() == 0 ?
						bms::EncodingType::SHIFT_JIS : bms::EncodingType::EUC_KR;
			}

			return jws.size() == 0 ? bms::EncodingType::EUC_KR :
									 bms::EncodingType::SHIFT_JIS;
		}

		/// <summary>
		/// Function that returns the number of beats of a particular measure
		/// stored in <see cref="mDicTimeSignature"/> of <see cref="BMSData"/> object.
		/// </summary>
		inline BeatFraction GetBeatInMeasure(int measure) {
			return mDicMeasureLength.count(measure) == 0 ? BeatFraction(4, 1) : mDicMeasureLength[measure] * 4;
		}
		/// <summary>
		/// Function that returns cumulative number of beats at a specific timing
		/// </summary>
		inline BeatFraction GetBeats(int measure, const BeatFraction& frac) {
			if (measure > mEndMeasure) {
				return BeatFraction();
			} else {
				return measure == 0 ? frac * GetBeatInMeasure(measure) :
									  frac * GetBeatInMeasure(measure) + mData.mListCumulativeBeat[measure - 1];
			}
		}
		/// <summary>
		/// Function that returns a time at a specific point using beats.
		/// note : Functions that convert time to beats are not provided because errors can occur during casting.
		/// </summary>
		inline long long GetTimeUsingBeat(const BeatFraction& beat) {
			BeatFraction subtract;
			size_t length = mData.mListTimeSeg.size();
			size_t index = length - 1;
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
			//TRACE("GetTimeUsingBeat beat area check : [" << (beat > prev.mCurBeat ? 1 : 0) << ", " << 
			//	  (index + 1 < length) ? (beat < mData.mListTimeSeg[index + 1].mCurBeat ? 1 : 0) : 1 << "], prevbpm : " << prev.mCurBpm)
			//bool bTest = (index + 1 < length) ? (mData.mListTimeSeg[index + 1].mCurBeat - beat > 0) : true;
			//std::cout << "GetTimeUsingBeat beat area check : [" << (beat - prev.mCurBeat >= 0) << ", " << bTest << "], prevbpm : " << prev.mCurBpm << std::endl;
			return index == 0 ? beat.GetTime(prev.mCurBpm) :
								prev.mCurTime + subtract.GetTime(prev.mCurBpm);
		}

		/// <summary>
		/// Function that returns a total play time
		/// </summary>
		inline long long GetTotalPlayTime() {
			const TimeSegment& seg = mData.mListTimeSeg[mData.mListTimeSeg.size() - 1];
			BeatFraction subtract = mData.mListCumulativeBeat[mEndMeasure] - seg.mCurBeat;

			return seg.mCurTime + subtract.GetTime(seg.mCurBpm);
		}

	private:
		BMSData& mData;

		/// <summary> The number of total measure of current bms data </summary>
		uint16_t mEndMeasure;
		/// <summary> 
		/// if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, 
		/// this value direct wav file key in <see cref="BMSData::mDicWav"/>. 
		/// </summary>
		uint16_t mEndNoteVal;

		///<summary> a list of temporary time data objects </summary>
		std::vector<Object> mListRawTiming;

		///<summary> a map of data objects (smallest unit) </summary>
		std::unordered_map<int, std::vector<Object>> mDicObj;
		///<summary> a list of data objects (smallest unit), the index is measure number </summary>
		std::vector<std::vector<Object>> mListObj;

		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, int> mDicStop;
		///<summary> pair of BPM command number and data  </summary>
		std::unordered_map<int, float> mDicBpm;
		///<summary> pair of measure number and measure length </summary>
		std::unordered_map<int, BeatFraction> mDicMeasureLength;

		///<summary> a list of STOP command data, the index is STOP command number </summary>
		int* mListStop;
		///<summary> a list of BPM command data, the index is BPM command number </summary>
		float* mListBpm;
		///<summary> a list of measure length, the index is measure number </summary>
		std::vector<BeatFraction> mListMeasureLength;

		/// <summary> parse function to change the value between "00" and "zz" to integer base 36 with no error check </summary>
		inline uint16_t ParseValue(const char* val) noexcept {
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
				acc = acc * 36 + c;
				c = *++val;
			} while (bFirstLoop != bFirstLoop);

			return acc;
		}
	};
}