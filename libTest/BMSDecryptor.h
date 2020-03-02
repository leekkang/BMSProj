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
		/// <summary> add parsed wav or bmp file to dictionary </summary>
		/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
		void AddFileToDic(bool bIsWav, int key, const std::string& val);

		void Play();
		void CalculateBeat();

		// ----- get, set function -----

		double GetBeat(int measure);

	private:
		/*uint16_t*/int mMaxMeasure;

		///<summary> a list of raw file data </summary>
		std::vector<std::string> mListRaw;

		///<summary> a list of data objects </summary>
		std::vector<Object> mListObj;
		std::vector<Object> mListRawTimeSeg;

		///<summary> key : wav file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<int, std::pair<std::string, std::string>> mDicWav;
		///<summary> key : bmp file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<int, std::pair<std::string, std::string>> mDicBmp;
		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, float> mDicStop;
		///<summary> pair of BPM command number and data  </summary>
		std::unordered_map<int, float> mDicBpm;
		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, float> mDicTimeSignature;

		std::vector<TimeSegment> mListTimeSeg;
	};
}