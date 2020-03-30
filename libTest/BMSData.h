#pragma once

#include "BMSObjects.h"

namespace bms {
	constexpr uint16_t MAX_INDEX_LENGTH = 1296;		// 00~ZZ, 36 * 36

	/// <summary>
	/// a data structure include information of <see cref="mPath"/> file for write UI information
	/// contains sorting information and minimal information to help you read the file.
	/// </summary>
	struct BMSInfoData {
		std::wstring mPath;
		EncodingType mFileType;

		uint8_t mPlayer;				// single = 1, 2p = 2, double = 3 (not implemented)
		uint8_t mLevel;					// music level ( 1 ~ 99 )
		uint8_t mDifficulty;			// easy,beginner,light = 1, normal,standard = 2, hard,hyper = 23, ex,another = 4, insane = 5
										// if this file hasn't difficulty header, default value is zero(undefined)

		uint16_t mNoteCount;			// the total number of normal note (filled after preview)
		uint64_t mTotalTime;			// the total play time (filled after preview)
		double mBpm;					// beats per minute (calculated and filled after the preview)
		double mMinBpm;					// max bpm at variable bpm
		double mMaxBpm;					// min bpm at variable bpm

		std::string mTitle;
		std::string mArtist;
		std::string mGenre;

		// -- additional BMSInfo (for construct preview data)

		uint16_t mWavCount;				// total wav file count
		uint16_t mMeasureCount;			// total measure count
		uint8_t mRndCount;				// total #RANDOM line count

		// ----- constructor, operator overloading -----

		BMSInfoData() = default;
		BMSInfoData(const std::wstring& path) : mPlayer(1), mLevel(0), mDifficulty(0), 
											   mNoteCount(0), mTotalTime(0), mMinBpm(0), mMaxBpm(0) {
			std::cout << "BMSData constructor" << std::endl;

			// Do not use it if class contains pointer variables.
			// I don't know why below link throw an error that says an access violation.
			// reference : https://www.sysnet.pe.kr/2/0/4
			//memset(((char *)this) + 4, 0, sizeof(BMSData) - 4);
			// Do not use it if class contains std::vector..!
			//memset(this, 0, sizeof(BMSData));

			mPath = path;
		}
		~BMSInfoData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSInfoData)
		BMSInfoData(BMSInfoData&& others) noexcept { std::cout << "BMSInfoData move constructor" << std::endl; *this = std::move(others); }
		BMSInfoData& operator=(BMSInfoData&&) noexcept = default;

		// ----- user access function -----

		friend std::ostream& operator<<(std::ostream& os, const BMSInfoData& s) {
			WriteToBinary(os, s.mPath);
			WriteToBinary(os, static_cast<uint8_t>(s.mFileType));
			WriteToBinary(os, s.mPlayer);
			WriteToBinary(os, s.mLevel);
			WriteToBinary(os, s.mDifficulty);
			WriteToBinary(os, s.mNoteCount);
			WriteToBinary(os, s.mTotalTime);
			WriteToBinary(os, s.mBpm);
			WriteToBinary(os, s.mMinBpm);
			WriteToBinary(os, s.mMaxBpm);
			WriteToBinary(os, s.mTitle);
			WriteToBinary(os, s.mArtist);
			WriteToBinary(os, s.mGenre);
			WriteToBinary(os, s.mWavCount);
			WriteToBinary(os, s.mMeasureCount);
			WriteToBinary(os, s.mRndCount);
			
			return os;
		}
		friend std::istream& operator>>(std::istream& is, BMSInfoData& s) {
			s.mPath = ReadFromBinary<std::wstring>(is);
			s.mFileType = static_cast<EncodingType>(ReadFromBinary<uint8_t>(is));
			s.mPlayer = ReadFromBinary<uint8_t>(is);
			s.mLevel = ReadFromBinary<uint8_t>(is);
			s.mDifficulty = ReadFromBinary<uint8_t>(is);
			s.mNoteCount = ReadFromBinary<uint16_t>(is);
			s.mTotalTime = ReadFromBinary<uint64_t>(is);
			s.mBpm = ReadFromBinary<double>(is);
			s.mMinBpm = ReadFromBinary<double>(is);
			s.mMaxBpm = ReadFromBinary<double>(is);
			s.mTitle = ReadFromBinary<std::string>(is);
			s.mArtist = ReadFromBinary<std::string>(is);
			s.mGenre = ReadFromBinary<std::string>(is);
			s.mWavCount = ReadFromBinary<uint16_t>(is);
			s.mMeasureCount = ReadFromBinary<uint16_t>(is);
			s.mRndCount = ReadFromBinary<uint8_t>(is);

			return is;
		}
	};

	/// <summary>
	/// a data structure include information of <see cref="mPath"/> file for preview music
	/// this class has <see cref="BMS::SongInfo"/> object and other data lists as member
	/// </summary>
	struct BMSPreviewData {
	public:
		// ----- constructor, operator overloading -----

		BMSPreviewData(const std::wstring& path) : mPlayer(1), mRank(2), mTotal(200), mLongNoteType(LongnoteType::RDM_TYPE_1) {
			std::cout << "BMSData constructor" << std::endl;

			// Do not use it if class contains pointer variables.
			// I don't know why below link throw an error that says an access violation.
			// reference : https://www.sysnet.pe.kr/2/0/4
			//memset(((char *)this) + 4, 0, sizeof(BMSData) - 4);
			// Do not use it if class contains std::vector..!
			//memset(this, 0, sizeof(BMSData));

			mPath = path;
		}
		~BMSPreviewData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSPreviewData)
		BMSPreviewData(BMSPreviewData&& others) noexcept { std::cout << "BMSPreviewData move constructor" << std::endl; *this = std::move(others); }
		BMSPreviewData& operator=(BMSPreviewData&&) noexcept = default;

		// ----- user access function -----

		friend std::ostream& operator<<(std::ostream& os, const BMSPreviewData& s) {
			WriteToBinary(os, s.mPlayer);
			WriteToBinary(os, s.mLevel);
			WriteToBinary(os, s.mRank);
			WriteToBinary(os, s.mDifficulty);
			WriteToBinary(os, s.mTotal);
			WriteToBinary<LongnoteType>(os, s.mLongNoteType);
			WriteToBinary(os, s.mNoteCount);
			WriteToBinary(os, s.mLongCount);
			WriteToBinary(os, s.mTotalTime);
			WriteToBinary(os, s.mBpm);
			WriteToBinary(os, s.mMinBpm);
			WriteToBinary(os, s.mMaxBpm);
			WriteToBinary(os, s.mGenre);
			WriteToBinary(os, s.mTitle);
			WriteToBinary(os, s.mArtist);
			WriteToBinary(os, s.mStageFile);
			WriteToBinary(os, s.mBannerFile);
			WriteToBinary(os, s.mPath);
			//WriteToBinary(os, s.mListSound);

			return os;
		}
		friend std::istream& operator>>(std::istream& is, BMSPreviewData& s) {
			s.mPlayer = ReadFromBinary<uint8_t>(is);
			s.mLevel = ReadFromBinary<uint8_t>(is);
			s.mRank = ReadFromBinary<uint8_t>(is);
			s.mDifficulty = ReadFromBinary<uint8_t>(is);
			s.mTotal = ReadFromBinary<uint16_t>(is);
			s.mLongNoteType = ReadFromBinary<LongnoteType>(is);
			s.mNoteCount = ReadFromBinary<uint16_t>(is);
			s.mLongCount = ReadFromBinary<uint16_t>(is);
			s.mTotalTime = ReadFromBinary<uint64_t>(is);
			s.mBpm = ReadFromBinary<double>(is);
			s.mMinBpm = ReadFromBinary<double>(is);
			s.mMaxBpm = ReadFromBinary<double>(is);
			s.mGenre = ReadFromBinary<std::string>(is);
			s.mTitle = ReadFromBinary<std::string>(is);
			s.mArtist = ReadFromBinary<std::string>(is);
			s.mStageFile = ReadFromBinary<std::string>(is);
			s.mBannerFile = ReadFromBinary<std::string>(is);
			s.mPath = ReadFromBinary<std::wstring>(is);
			//ReadFromBinary<std::vector<std::pair<uint64_t, uint16_t>>>(is, s.mListSound);

			return is;
		}

		uint8_t mPlayer;				// single = 1, 2p = 2, double = 3 (not implemented)
		uint8_t mLevel;					// music level ( 1 ~ 99 )
		uint8_t mRank;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		uint8_t mDifficulty;			// easy,beginner,light = 0, normal,standard = 1, hard,hyper = 2, ex,another = 3, insane = 4
		uint16_t mTotal;				// guage value
		LongnoteType mLongNoteType;		// longnote type.

		uint16_t mNoteCount;			// the total number of normal note
		uint16_t mLongCount;			// the total number of long note

		uint64_t mTotalTime;			// the total play time
		double mBpm;					// beats per minute
		double mMinBpm;					// max bpm at variable bpm
		double mMaxBpm;					// min bpm at variable bpm

		std::string mGenre;
		std::string mTitle;
		std::string mArtist;

		std::string mStageFile;			// loading image file name when music ready
		std::string mBannerFile;		// banner image file name of inform music or team

		std::wstring mPath;

		///<summary> a list to play preview. The pair consists of time and music file index </summary>
		std::vector<std::pair<uint64_t, uint16_t>> mListSound;
	};

	/// <summary>
	/// a data structure include all information of <see cref="mPath"/> file for game play
	/// this class has <see cref="BMS::BMSPreviewData"/> object and other data lists as member
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