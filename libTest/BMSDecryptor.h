#pragma once

#include "Utility.h"
#include "BMSEnums.h"
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

		void Play();
		/// <summary> add parsed wav or bmp file to dictionary </summary>
		/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
		void AddFileToDic(bool bIsWav, int key, const std::string& val);

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
		inline double GetTimeUsingBeat(const BeatFraction& beat) {
			BeatFraction subtract;
			for (const TimeSegment& t : mBmsData.mListTimeSeg) {
				subtract = t.mCurBeat - beat;
				// zero bpm means stop signal. skip it.
				if (subtract >= 0 && t.mCurBpm != 0) {
					// previous saved time + current segment time
					return t.mCurTime + subtract.GetTime(t.mCurBpm);
				}
			}

			const TimeSegment& last = mBmsData.mListTimeSeg[mBmsData.mListTimeSeg.size() - 1];
			return last.mCurTime - subtract.GetTime(last.mCurBpm);
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