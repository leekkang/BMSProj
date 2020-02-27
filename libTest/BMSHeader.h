#pragma once

#include <string>
#include <deque>
#include <unordered_map>

namespace BMS {
	class BMSHeader {
	public:
		BMSHeader() {}
		~BMSHeader() {}

		/// <summary> add parsed wav or bmp file to dictionary </summary>
		/// <param name="bIsWav"> flag whether this file is wav or bmp </param>
		void AddFileToDict(bool bIsWav, const std::string& key, const std::string& val);

		///<summary> add parsed stop or bpm command to list </summary>
		/// <param name="bIsWav"> flag whether this command is stop or bpm </param>
		void AddCmdToList(bool bIsStop, const std::string& index, const std::string& val);

		int GetPlayer() { return mPlayer; }
		int GetBpm() { return mBpm; }
		int GetLevel() { return mLevel; }
		int GetRank() { return mRank; }
		int GetTotal() { return mTotal; }
		int GetDifficulty() { return mDifficulty; }
		std::string GetGenre() { return mGenre; }
		std::string GetTitle() { return mTitle; }
		std::string GetArtist() { return mArtist; }
		std::string GetStageFile() { return mStageFile; }
		std::string GetBanner() { return mBannerFile; }

		void SetPlayer(int val) { mPlayer = val; }
		void SetBpm(int val) { mBpm = val; }
		void SetLevel(int val) { mLevel = val; }
		void SetRank(int val) { mRank = val; }
		void SetTotal(int val) { mTotal = val; }
		void SetDifficulty(int val) { mDifficulty = val; }
		void SetGenre(std::string val) { mGenre = val; }
		void SetTitle(std::string val) { mTitle = val; }
		void SetArtist(std::string val) { mArtist = val; }
		void SetStageFile(std::string val) { mStageFile = val; }
		void SetBanner(std::string val) { mBannerFile = val; }

	private:
		int mPlayer = 0;				// single = 1, 2p = 2, double = 3 (not implemented)
		int mBpm = 0;					// beats per minute
		int mLevel = 0;					// music level ( 1 ~ 99 )
		int mRank = 2;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		int mTotal = 200;				// guage value
		int mDifficulty = 0;			// easy = 0, normal = 1, hard = 2, extra = 3, another = 4

		std::string mGenre = "";
		std::string mTitle = "";
		std::string mArtist = "";

		std::string mStageFile = "";	// loading image file name when music ready
		std::string mBannerFile = "";	// banner image file name of inform music or team

		///<summary> key : wav file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<std::string, std::pair<std::string, std::string>> mDicWav;
		///<summary> key : bmp file mapping value, value : pair of wav file name and extension </summary>
		std::unordered_map<std::string, std::pair<std::string, std::string>> mDicBmp;
		///<summary> pair of STOP command number and data </summary>
		std::deque <std::pair<std::string, std::string>> mListStop;
		///<summary> pair of BPM command number and data  </summary>
		std::deque <std::pair<std::string, std::string>> mListBpm;
	};
}