#pragma once

#include "BMSObjects.h"

namespace bms {
	constexpr uint16_t MAX_INDEX_LENGTH = 1296;		// 00~ZZ, 36 * 36

	/// <summary>
	/// a data structure include information of <see cref="mPath"/> file for write UI information
	/// contains sorting information and minimal information to help you read the file.
	/// </summary>
	struct BMSInfoData {
		std::wstring mFilePath;
		EncodingType mFileType;

		KeyType mKeyType;				// determine how many keys the file uses.
		uint8_t mLevel;					// music level ( 1 ~ 99 )
		uint8_t mDifficulty;			// easy,beginner,light = 1, normal,standard = 2, hard,hyper = 3, ex,another = 4, insane = 5
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

		bool mHasRandom;				// total #RANDOM line count
		uint16_t mWavCount;				// The number of total wav file
		uint16_t mMeasureCount;			// The number of total measure of current bms data

		// non-save data (when filesystem check, this value is filled)
		/// <summary> Defined for confirmation because the extension of the music list written in the bms file may be different due to its capacity. </summary>
		std::string mSoundExtension;

		// ----- constructor, operator overloading -----

		BMSInfoData() : mLevel(0), mDifficulty(0),
						mNoteCount(0), mTotalTime(0), mMinBpm(0), mMaxBpm(0) {
			// Do not use it if class contains pointer variables.
			// I don't know why below link throw an error that says an access violation.
			// reference : https://www.sysnet.pe.kr/2/0/4
			//memset(((char *)this) + 4, 0, sizeof(BMSData) - 4);
			// Do not use it if class contains std::vector..!
			//memset(this, 0, sizeof(BMSData));
		}
		~BMSInfoData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSInfoData)
		BMSInfoData(BMSInfoData&& others) noexcept { std::cout << "BMSInfoData move constructor" << std::endl; *this = std::move(others); }
		BMSInfoData& operator=(BMSInfoData&&) noexcept = default;

		// ----- user access function -----

		friend std::ostream& operator<<(std::ostream& os, const BMSInfoData& s) {
			WriteToBinary(os, s.mFilePath);
			WriteToBinary(os, static_cast<uint8_t>(s.mFileType));
			WriteToBinary(os, static_cast<uint8_t>(s.mKeyType));
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
			WriteToBinary(os, s.mHasRandom);
			
			return os;
		}
		friend std::istream& operator>>(std::istream& is, BMSInfoData& s) {
			s.mFilePath = ReadFromBinary<std::wstring>(is);
			s.mFileType = static_cast<EncodingType>(ReadFromBinary<uint8_t>(is));
			s.mKeyType = static_cast<KeyType>(ReadFromBinary<uint8_t>(is));
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
			s.mHasRandom = ReadFromBinary<bool>(is);

			return is;
		}
	};

	/// <summary>
	/// a data structure include all information of <see cref="mPath"/> file for game play
	/// this class has <see cref="BMS::BMSPreviewData"/> object and other data lists as member
	/// </summary>
	struct BMSData {
	public:
		// ----- constructor, operator overloading -----

		BMSData() : mInfo(nullptr), mReady(false), mRank(2), mTotal(200), mLongNoteType(LongnoteType::RDM_TYPE_1) {
			mListWavName = new std::string[MAX_INDEX_LENGTH];
			mListBmpName = new std::string[MAX_INDEX_LENGTH];
		}
		~BMSData() {
			delete[] mListWavName;
			delete[] mListBmpName;
		}
		DISALLOW_COPY_AND_ASSIGN(BMSData)
		//BMSData(const BMSData&) = default;
		//BMSData& operator=(const BMSData&) = default;
		BMSData(BMSData&& others) noexcept = default;// { std::cout << "BMSData move constructor" << std::endl; *this = std::move(others); }
		BMSData& operator=(BMSData&&) noexcept = default;

		// ----- user access function -----

		///<summary> reset all member variable </summary>
		void Reset(BMSInfoData* const& info, bool bPreview) {
			mReady = false;
			mInfo = info;

			mNoteCount = 0;
			mLongCount = 0;
			mListTimeSeg.clear();
			mListBga.clear();
			mListBgm.clear();
			mListPlayerNote.clear();

			for (uint16_t i = 0; i < MAX_INDEX_LENGTH; ++i) {
				mListWavName[i].clear();
				if (!bPreview) {
					mListBmpName[i].clear();
				}
			}

			uint16_t measureCnt = info->mMeasureCount;
			if (mListCumulativeBeat.size() < measureCnt) {
				mListCumulativeBeat.resize(measureCnt);
			}
		}

		BMSInfoData* mInfo;

		bool mReady;				// check if build is complete

		int mRank;					// judgement line difficulty. easy = 3, normal = 2, hard = 1, very hard = 0
		int mTotal;					// guage value
		LongnoteType mLongNoteType;	// longnote type.

		int mNoteCount;				// the total number of normal note
		int mLongCount;				// the total number of long note

		std::string mStageFile;		// loading image file name when music ready
		std::string mBannerFile;	// banner image file name of inform music or team

		///<summary> a list of wav file name, the index is wav file mapping value </summary>
		std::string* mListWavName;
		///<summary> a list of bmp or video file name, the index is bmp file mapping value </summary>
		std::string* mListBmpName;

		///<summary> a list of the cumulative number of bits per measure  </summary>
		std::vector<BeatFraction> mListCumulativeBeat;
		///<summary> a list of the change timing point  </summary>
		ListPool<TimeSegment> mListTimeSeg;
		///<summary> a list of the note include BGA data  </summary>
		ListPool<Note> mListBga;
		///<summary> a list of the note include BGM data  </summary>
		ListPool<Note> mListBgm;
		///<summary> a list of the note that shown on the track </summary>
		ListPool<PlayerNote> mListPlayerNote;
	};
}