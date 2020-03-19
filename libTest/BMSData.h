#pragma once

#include "BMSObjects.h"

namespace bms {
	/// <summary>
	/// a data structure include all information of <see cref="mPath"/> file
	/// this class has <see cref="BMS::SongInfo"/> object and other data lists as member
	/// </summary>
	struct BMSData {
	public:
		// ----- constructor, operator overloading -----

		BMSData(const std::string& path) : mPlayer(1), mRank(2), mTotal(200), mLongNoteType(LongnoteType::RDM_TYPE_1) {
			std::cout << "BMSData constructor" << std::endl;

			// Do not use it if class contains pointer variables.
			// I don't know why below link throw an error that says an access violation.
			// reference : https://www.sysnet.pe.kr/2/0/4
			//memset(((char *)this) + 4, 0, sizeof(BMSData) - 4);
			// Do not use it if class contains std::vector..!
			//memset(this, 0, sizeof(BMSData));

			mPath = path;
		}
		~BMSData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSData)
		//BMSData(const BMSData&) = default;
		//BMSData& operator=(const BMSData&) = default;
		BMSData(BMSData&& others) noexcept { std::cout << "BMSData move constructor" << std::endl; *this = std::move(others); }
		BMSData& operator=(BMSData&&) noexcept = default;

		// ----- user access function -----

		friend std::ostream& operator<<(std::ostream& os, const BMSData& s) {
			WriteToBinary(os, s.mPlayer);
			WriteToBinary(os, s.mLevel);
			WriteToBinary(os, s.mRank);
			WriteToBinary(os, s.mDifficulty);
			WriteToBinary(os, s.mTotal);
			WriteToBinary(os, s.mNoteCount);
			WriteToBinary(os, s.mLongCount);
			WriteToBinary(os, s.mTotalTime);
			WriteToBinary(os, s.mBpm);
			WriteToBinary(os, s.mMinBpm);
			WriteToBinary(os, s.mMaxBpm);
			WriteToBinary<LongnoteType>(os, s.mLongNoteType);
			WriteToBinary(os, s.mGenre);
			WriteToBinary(os, s.mTitle);
			WriteToBinary(os, s.mArtist);
			WriteToBinary(os, s.mStageFile);
			WriteToBinary(os, s.mBannerFile);
			WriteToBinary(os, s.mPath);
			WriteToBinary(os, s.mDicWav);
			WriteToBinary(os, s.mDicBmp);
			WriteToBinary(os, s.mListCumulativeBeat);
			WriteToBinary(os, s.mListTimeSeg);
			WriteToBinary(os, s.mListBga);
			WriteToBinary(os, s.mListBgm);
			WriteToBinary(os, s.mListPlayerNote);

			return os;
		}
		friend std::istream& operator>>(std::istream& is, BMSData& s) {
			s.mPlayer = ReadFromBinary<int>(is);
			s.mLevel = ReadFromBinary<int>(is);
			s.mRank = ReadFromBinary<int>(is);
			s.mDifficulty = ReadFromBinary<int>(is);
			s.mTotal = ReadFromBinary<int>(is);
			s.mNoteCount = ReadFromBinary<int>(is);
			s.mLongCount = ReadFromBinary<int>(is);
			s.mTotalTime = ReadFromBinary<long long>(is);
			s.mBpm = ReadFromBinary<double>(is);
			s.mMinBpm = ReadFromBinary<double>(is);
			s.mMaxBpm = ReadFromBinary<double>(is);
			s.mLongNoteType = ReadFromBinary<LongnoteType>(is);
			s.mGenre = ReadFromBinary<std::string>(is);
			s.mTitle = ReadFromBinary<std::string>(is);
			s.mArtist = ReadFromBinary<std::string>(is);
			s.mStageFile = ReadFromBinary<std::string>(is);
			s.mBannerFile = ReadFromBinary<std::string>(is);
			s.mPath = ReadFromBinary<std::string>(is);
			ReadFromBinary<std::unordered_map<int, std::string>>(is, s.mDicWav);
			ReadFromBinary<std::unordered_map<int, std::string>>(is, s.mDicBmp);
			ReadFromBinary<std::vector<BeatFraction>>(is, s.mListCumulativeBeat);
			ReadFromBinary<std::vector<TimeSegment>>(is, s.mListTimeSeg);
			ReadFromBinary<std::vector<Note>>(is, s.mListBga);
			ReadFromBinary<std::vector<Note>>(is, s.mListBgm);
			ReadFromBinary<std::vector<PlayerNote>>(is, s.mListPlayerNote);

			return is;
		}

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