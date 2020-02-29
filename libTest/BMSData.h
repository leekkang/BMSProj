#pragma once

#include "Utility.h"
#include "BMSEnums.h"

#include <unordered_map>

namespace bms {
	/// <summary>
	/// a data structure represents an object inside a <see cref="BMSBody"/> class
	/// smallest unit in this data. music only + note + option
	/// </summary>
	class Object {
	public:
		Object(int measure, Channel channel, int fracIndex, int fracDenom, int val) :
			mMeasure(measure), mChannel(channel), mFractionIndex(fracIndex), mFractionDenom(fracDenom), mValue(val) {}
		~Object() = default;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

		/// <summary>
		/// check <paramref name="other"/> has equal measure, channel, fraction
		/// </summary>
		bool IsSameBeat(Object& other) {
			return other.mMeasure == mMeasure && other.mChannel == mChannel &&
				other.mFractionIndex == mFractionIndex && other.mFractionDenom == mFractionDenom;
		}

		// ----- get, set function -----

		int GetMeasure() { return mMeasure; }
		Channel GetChannel() { return mChannel; }
		int GetFracIndex() { return mFractionIndex; }
		int GetFracDenom() { return mFractionDenom; }
		int GetValue() { return mValue; }

		//void SetValue(int val) { mValue = val; }

	private:
		int mMeasure;			// the measure number, starting at 0 (corresponds to `#000`)
		Channel mChannel;		// value of Channel enum
		int mFractionIndex;		// numerator of the fractional position inside the measure
		int mFractionDenom;		// denominator of the fractional position inside the measure
		int mValue;				// the raw value of the BMS object
	};

	/// <summary>
	/// a data structure include all information of <see cref="mPath"/> file
	/// this class has <see cref="BMS::SongInfo"/> object and other data lists as member
	/// </summary>
	class BMSData {
	public:
		// ----- constructor, operator overloading -----

		BMSData() { std::cout << "BMSData constructor" << std::endl; };
		~BMSData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSData)
		//BMSData(const BMSData&) = delete;
		//BMSData& operator=(const BMSData&) = delete;
		BMSData(BMSData&& others) noexcept { std::cout << "BMSData move constructor" << std::endl; *this = std::move(others); }
		BMSData& operator=(BMSData&&) noexcept = default;

		// ----- user access function -----

		/// <summary>
		/// build using line in <paramref name="lines"/> list for fill data in header or body
		/// </summary>
        /// <returns> return true if all line is correctly saved </returns>
		bool Build(std::vector<std::string>& lines);
		/// <summary>
		/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
		/// </summary>
		void ParseHeader(std::string&& line) noexcept;
		/// <summary>
		/// parse <paramref name="line"/> for fill body data and store parsed line in appropriate variable
		/// </summary>
		void ParseBody(std::string&& line) noexcept;
		/// <summary> add parsed wav or bmp file to dictionary </summary>
		/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
		void AddFileToDic(bool bIsWav, int key, const std::string& val);

		void Play();
		void CalculateBeat();

		// ----- get, set function -----

		int GetPlayer() { return mPlayer; }
		int GetBpm() { return mBpm; }
		int GetLevel() { return mLevel; }
		int GetRank() { return mRank; }
		int GetTotal() { return mTotal; }
		int GetDifficulty() { return mDifficulty; }
		LongnoteType GetLongNoteType() { return mLongNoteType; }
		const std::string& GetGenre() { return mGenre; }
		const std::string& GetTitle() { return mTitle; }
		const std::string& GetArtist() { return mArtist; }
		const std::string& GetStageFile() { return mStageFile; }
		const std::string& GetBanner() { return mBannerFile; }

		const std::string& GetFilePath() { return mPath; }

		void SetFilePath(std::string& path) { mPath = path; }

	private:
		int mPlayer = 0;				// single = 1, 2p = 2, double = 3 (not implemented)
		int mBpm = 0;					// beats per minute
		int mLevel = 0;					// music level ( 1 ~ 99 )
		int mRank = 2;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		int mDifficulty = 0;			// easy,beginner,light = 0, normal,standard = 1, hard,hyper = 2, ex,another = 3, insane = 4
		int mTotal = 200;				// guage value
		int mEndNoteVal = 0;			/// <summary> if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, this value direct wav file key in <see cref="mDicWav"/>. </summary>
		LongnoteType mLongNoteType = LongnoteType::RDM_TYPE_1;		// longnote type.

		std::string mGenre = "";
		std::string mTitle = "";
		std::string mArtist = "";

		std::string mStageFile = "";	// loading image file name when music ready
		std::string mBannerFile = "";	// banner image file name of inform music or team

		std::string mPath;

		///<summary> a list of data objects </summary>
		std::vector<Object> mListObj;

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

	};
}