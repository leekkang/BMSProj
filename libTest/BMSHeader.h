#pragma once

#include "Utility.h"
#include "BMSEnums.h"

#include <string>
#include <unordered_map>

namespace bms {
	class BMSHeader {
	public:
		BMSHeader() { std::cout << "BMSHeader constructor" << std::endl; };
		~BMSHeader() = default;

		BMSHeader(const BMSHeader&) = delete;
		BMSHeader& operator=(const BMSHeader&) = delete;
		BMSHeader(BMSHeader&& others) { *this = std::move(others); }
		BMSHeader& operator=(BMSHeader&&) = default;

		void StoreHeader(std::string&& line);

		/// <summary> add parsed wav or bmp file to dictionary </summary>
		/// <param name="bIsWav"/> flag whether this file is wav or bmp </param>
		void AddFileToDic(bool bIsWav, int key, const std::string& val);

		int GetPlayer() { return mPlayer; }
		int GetBpm() { return mBpm; }
		int GetLevel() { return mLevel; }
		int GetRank() { return mRank; }
		int GetTotal() { return mTotal; }
		int GetDifficulty() { return mDifficulty; }
		int GetLongNoteType() { return mLongNoteType; }
		const std::string& GetGenre() { return mGenre; }
		const std::string& GetTitle() { return mTitle; }
		const std::string& GetArtist() { return mArtist; }
		const std::string& GetStageFile() { return mStageFile; }
		const std::string& GetBanner() { return mBannerFile; }

		/*void SetPlayer(int val) { mPlayer = val; }
		void SetBpm(int val) { mBpm = val; }
		void SetLevel(int val) { mLevel = val; }
		void SetRank(int val) { mRank = val; }
		void SetTotal(int val) { mTotal = val; }
		void SetDifficulty(int val) { mDifficulty = val; }
		void SetLongNoteType(int val) { mLongNoteType = val; }
		void SetGenre(std::string&& val) { mGenre = val; }
		void SetTitle(std::string&& val) { mTitle = val; }
		void SetArtist(std::string&& val) { mArtist = val; }
		void SetStageFile(std::string&& val) { mStageFile = val; }
		void SetBanner(std::string&& val) { mBannerFile = val; }*/

	private:
		int mPlayer = 0;				// single = 1, 2p = 2, double = 3 (not implemented)
		int mBpm = 0;					// beats per minute
		int mLevel = 0;					// music level ( 1 ~ 99 )
		int mRank = 2;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		int mTotal = 200;				// guage value
		int mDifficulty = 0;			// easy,beginner,light = 0, normal,standard = 1, hard,hyper = 2, ex,another = 3, insane = 4
		int mLongNoteType = static_cast<int>(LongnoteType::RDM_TYPE_1);		// longnote type.
		int mEndNoteVal = 0;			/// <summary> if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, this value direct wav file index. </summary>

		std::string mGenre = "";
		std::string mTitle = "";
		std::string mArtist = "";

		std::string mStageFile = "";	// loading image file name when music ready
		std::string mBannerFile = "";	// banner image file name of inform music or team

		///<summary> key : wav file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<int, std::pair<std::string, std::string>> mDicWav;
		///<summary> key : bmp file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<int, std::pair<std::string, std::string>> mDicBmp;
		///<summary> pair of STOP command number and data </summary>
		std::unordered_map <int, float> mDicStop;
		///<summary> pair of BPM command number and data  </summary>
		std::unordered_map <int, float> mDicBpm;
	};
}