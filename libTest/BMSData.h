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
	struct BMSData {
	public:
		// ----- constructor, operator overloading -----

		BMSData(std::string path);
		~BMSData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSData)
		//BMSData(const BMSData&) = delete;
		//BMSData& operator=(const BMSData&) = delete;
		BMSData(BMSData&& others) noexcept { std::cout << "BMSData move constructor" << std::endl; *this = std::move(others); }
		BMSData& operator=(BMSData&&) noexcept = default;

		// ----- user access function -----



		int mPlayer;				// single = 1, 2p = 2, double = 3 (not implemented)
		int mLevel;					// music level ( 1 ~ 99 )
		int mRank;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		int mDifficulty;			// easy,beginner,light = 0, normal,standard = 1, hard,hyper = 2, ex,another = 3, insane = 4
		int mTotal;					// guage value
		LongnoteType mLongNoteType;	// longnote type.

		int mNoteCount;				// the total number of normal note
		int mLongCount;				// the total number of long note

		long long mTotalTime;		// the total play time
		double mBpm;				// beats per minute
		double mMinBpm;				// max bpm at variable bpm
		double mMaxBpm;				// min bpm at variable bpm

		std::string mGenre;
		std::string mTitle;
		std::string mArtist;

		std::string mStageFile;		// loading image file name when music ready
		std::string mBannerFile;	// banner image file name of inform music or team

		std::string mPath;

		///<summary> key : wav file mapping value, value : wav file name </summary>
		std::unordered_map<int, std::string> mDicWav;
		///<summary> key : bmp file mapping value, value : bmp or video file name </summary>
		std::unordered_map<int, std::string> mDicBmp;

		///<summary> a list of the cumulative number of bits per measure  </summary>
		std::vector<BeatFraction> mListCumulativeBeat;
		///<summary> a list of the change timing point  </summary>
		std::vector<TimeSegment> mListTimeSeg;
		///<summary> a list of the note include BGA data  </summary>
		std::vector<Note> mListBga;
		///<summary> a list of the note include BGM data  </summary>
		std::vector<Note> mListBgm;
		///<summary> a list of the note that shown on the track </summary>
		std::vector<PlayerNote> mListPlayerNote;
	};
}