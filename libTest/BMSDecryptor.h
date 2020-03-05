#pragma once

#include "BMSData.h"
#include "BMSObjects.h"

#include <unordered_map>
#include <algorithm>
#include <functional>

namespace bms {

	/// <summary>
	/// A data structure containing temporary variables and functions required for `bms data` calculations.
	/// This class is declared as a local variable and will not be saved after the whole process.
	/// </summary>
	class BMSDecryptor {
	public:
		// ----- constructor, operator overloading -----

		BMSDecryptor(std::string path, std::vector<std::string>& data) : mBmsData(path) {
			std::cout << "BMSDecryptor constructor" << std::endl; 
			mListRaw = data;
		};
		~BMSDecryptor() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSDecryptor)
		BMSDecryptor(BMSDecryptor&& others) noexcept = default;
		BMSDecryptor& operator=(BMSDecryptor&&) noexcept = default;

		// ----- user access function -----

		/// <summary>
		/// build using line in <paramref name="lines"/> list for fill data in header or body
		/// </summary>
		/// <returns> return true if all line is correctly saved </returns>
		bool Build();
		/// <summary>
		/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
		/// </summary>
		void ParseHeader(std::string&& line) noexcept;
		/// <summary>
		/// parse <paramref name="line"/> for fill body data and store parsed line in temporary data structure
		/// </summary>
		void ParseBody(std::string&& line) noexcept;
		/// <summary>
		/// make time segment list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::TimeSegment"/> objects
		/// </summary>
		void MakeTimeSegment();
		/// <summary>
		/// make note list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::Note"/> objects
		/// </summary>
		void MakeNoteList();
		void MakeNoteList2();

		// ----- get, set function -----

		/// <summary>
		/// return <see cref="BMSData"/> object. It should be called after the build is finished.
		/// </summary>
		inline BMSData&& GetBmsData() { return std::move(mBmsData); }

		/// <summary>
		/// Function that returns the number of beats of a particular measure
		/// stored in <see cref="mDicTimeSignature"/> of <see cref="BMSData"/> object.
		/// </summary>
		inline BeatFraction GetBeatInMeasure(int measure) {
			return mDicTimeSignature.count(measure) == 0 ? BeatFraction(4, 1) : mDicTimeSignature[measure] * 4;
		}
		/// <summary>
		/// Function that returns cumulative number of beats at a specific timing
		/// </summary>
		inline BeatFraction GetBeats(int measure, const BeatFraction& frac) {
			if (measure > mEndMeasure) {
				return BeatFraction();
			} else {
				return measure == 0 ? frac * GetBeatInMeasure(measure) :
									  frac * GetBeatInMeasure(measure) + mBmsData.mListCumulativeBeat[measure - 1];
			}
		}
		/// <summary>
		/// Function that returns a time at a specific point using beats.
		/// note : Functions that convert time to beats are not provided because errors can occur during casting.
		/// </summary>
		inline long long GetTimeUsingBeat(const BeatFraction& beat) {
			BeatFraction subtract;
			int length = mBmsData.mListTimeSeg.size();
			int index = length - 1;
			for (; index > 0; --index) {
				const TimeSegment& t = mBmsData.mListTimeSeg[index];
				subtract = beat - t.mCurBeat;
				// zero bpm means stop signal. skip it.
				if (subtract >= 0) {
					break;
				}
			}

			// previous saved time + current segment time. if index equals to length, add reversed sign
			const TimeSegment& prev = mBmsData.mListTimeSeg[index];
			//TRACE("GetTimeUsingBeat beat area check : [" << (beat > prev.mCurBeat ? 1 : 0) << ", " << 
			//	  (index + 1 < length) ? (beat < mBmsData.mListTimeSeg[index + 1].mCurBeat ? 1 : 0) : 1 << "], prevbpm : " << prev.mCurBpm)
			//bool bTest = (index + 1 < length) ? (mBmsData.mListTimeSeg[index + 1].mCurBeat - beat > 0) : true;
			//std::cout << "GetTimeUsingBeat beat area check : [" << (beat - prev.mCurBeat >= 0) << ", " << bTest << "], prevbpm : " << prev.mCurBpm << std::endl;
			return index == 0 ? beat.GetTime(prev.mCurBpm) :
								prev.mCurTime + subtract.GetTime(prev.mCurBpm);
		}

		/// <summary>
		/// Function that returns a total play time
		/// </summary>
		inline long long GetTotalPlayTime() {
			const TimeSegment& seg = mBmsData.mListTimeSeg[mBmsData.mListTimeSeg.size() - 1];
			BeatFraction subtract = mBmsData.mListCumulativeBeat[mEndMeasure] - seg.mCurBeat;

			return seg.mCurTime + subtract.GetTime(seg.mCurBpm);
		}

	private:
		/// <summary> The number of total measure of current bms data </summary>
		int mEndMeasure;
		/// <summary> 
		/// if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, 
		/// this value direct wav file key in <see cref="BMSData::mDicWav"/>. 
		/// </summary>
		int mEndNoteVal;

		BMSData mBmsData;

		///<summary> a list of raw file data not yet parsed </summary>
		std::vector<std::string> mListRaw;
		///<summary> a list of temporary time data objects </summary>
		std::vector<Object> mListRawTimeSeg;

		///<summary> a map of data objects (smallest unit) </summary>
		std::unordered_map<int, std::vector<Object>> mDicObj;

		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, float> mDicStop;
		///<summary> pair of BPM command number and data  </summary>
		std::unordered_map<int, float> mDicBpm;
		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, BeatFraction> mDicTimeSignature;
	};
}