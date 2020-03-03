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

		BMSDecryptor(std::vector<std::string>& data) {
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
		bool Build(BMSData& bmsData);
		/// <summary>
		/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
		/// </summary>
		void ParseHeader(std::string&& line, BMSData& bmsData) noexcept;
		/// <summary>
		/// parse <paramref name="line"/> for fill body data and store parsed line in temporary data structure
		/// </summary>
		void ParseBody(std::string&& line) noexcept;
		/// <summary>
		/// make time segment list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::TimeSegment"/> objects
		/// </summary>
		void MakeTimeSegment(BMSData& bmsData);
		/// <summary>
		/// make note list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::Note"/> objects
		/// </summary>
		void MakeNoteList(BMSData& bmsData);

		void Play();
		void CalculateBeat();
		/// <summary> add parsed wav or bmp file to dictionary </summary>
		/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
		void AddFileToDic(bool bIsWav, int key, const std::string& val);

		// ----- get, set function -----

		/// <summary>
		/// get Fraction that stored in <see cref="mDicTimeSignature"/> of <see cref="BMSData"/> object.
		/// </summary>
		Utility::Fraction GetBeatInMeasure(int measure);
		/// <summary>
		/// Function that returns the total number of beats to a point
		/// </summary>
		Utility::Fraction GetBeats(int measure, const Utility::Fraction& frac);

	private:
		/*uint16_t*/int mEndMeasure;

		///<summary> a list of raw file data not yet parsed </summary>
		std::vector<std::string> mListRaw;

		///<summary> a list of data objects </summary>
		std::vector<Object> mListObj;
		///<summary> a list of temporary time data objects </summary>
		std::vector<Object> mListRawTimeSeg;

		std::unordered_map<int, std::vector<Object>> mDicObj;

		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, float> mDicStop;
		///<summary> pair of BPM command number and data  </summary>
		std::unordered_map<int, float> mDicBpm;
		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, Utility::Fraction> mDicTimeSignature;
	};
}