#pragma once

#include "Utility.h"
#include "BMSEnums.h"
#include "BMSObjects.h"

#include <unordered_map>

namespace bms {
	/// <summary>
	/// a data structure include all information of <see cref="mPath"/> file
	/// this class has <see cref="BMS::SongInfo"/> object and other data lists as member
	/// </summary>
	class BMSData {
	public:
		// ----- constructor, operator overloading -----

		BMSData(std::string);
		~BMSData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSData)
		//BMSData(const BMSData&) = delete;
		//BMSData& operator=(const BMSData&) = delete;
		BMSData(BMSData&& others) noexcept { std::cout << "BMSData move constructor" << std::endl; *this = std::move(others); }
		BMSData& operator=(BMSData&&) noexcept = default;

		// ----- user access function -----


		// ----- get, set function -----

		int GetPlayer() { return mPlayer; }
		double GetBpm() { return mBpm; }
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

	//private:
		int mPlayer;				// single = 1, 2p = 2, double = 3 (not implemented)
		int mLevel;					// music level ( 1 ~ 99 )
		int mRank;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		int mDifficulty;			// easy,beginner,light = 0, normal,standard = 1, hard,hyper = 2, ex,another = 3, insane = 4
		int mTotal;					// guage value
		int mEndNoteVal;			/// <summary> if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, this value direct wav file key in <see cref="mDicWav"/>. </summary>
		LongnoteType mLongNoteType;	// longnote type.

		double mBpm;				// beats per minute
		double mMinBpm;				// max bpm at variable bpm
		double mMaxBpm;				// min bpm at variable bpm

		std::string mGenre;
		std::string mTitle;
		std::string mArtist;

		std::string mStageFile;		// loading image file name when music ready
		std::string mBannerFile;	// banner image file name of inform music or team

		std::string mPath;

		///<summary> a list of data objects </summary>
		std::vector<Object> mListObj;

		///<summary> key : wav file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<int, std::pair<std::string, std::string>> mDicWav;
		///<summary> key : bmp file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<int, std::pair<std::string, std::string>> mDicBmp;

		///<summary> a list of the cumulative number of bits per measure  </summary>
		std::vector<Utility::Fraction> mListCumulativeBeat;
		///<summary> a list of the change timing point  </summary>
		std::vector<TimeSegment> mListTimeSeg;
	};
}