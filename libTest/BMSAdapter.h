#pragma once

#include "BMSDecryptor.h"
#include "BMSPlayThread.h"
#include "BMSTree.h"

namespace bms {
	/// <summary>
	/// A class that manages <see cref="bms::BMSData"/> and <see cref="bms::PlayTyread"/>
	/// Only a maximum of one thread can be created.
	/// </summary>
	class BMSAdapter {
	public:
		// ----- constructor, operator overloading -----

		BMSAdapter() : mDecryptor(mCurData), mThread(mCurData), mPathTree(mDecryptor) {
			Load();
		};
		~BMSAdapter() {
			Save();
		};
		DISALLOW_COPY_AND_ASSIGN(BMSAdapter)
		BMSAdapter(BMSAdapter&&) noexcept = default;
		BMSAdapter& operator=(BMSAdapter&&) noexcept = default;

		// ----- user access function -----

		/// <summary>
		/// check if pattern is exists, and call <see cref="Play(BMSInfoData* const&)"/> function
		/// </summary>
		void Play(uint16_t folderIndex, uint16_t musicIndex, uint8_t patternIndex) {
			BMSInfoData* data = mPathTree.GetPattern(folderIndex, musicIndex, patternIndex);
			if (data == nullptr) {
				LOG("There is no BMSData at the matching index : " << folderIndex << ", " << musicIndex << ", " << patternIndex);
				return;
			}
			Play(data);
		}

		/// <summary>
		/// Preview bms music using <paramref name="info"/> instance.
		/// if the previous data is different from info in <see cref="mCurData"/>, build a new one
		/// </summary>
		void Play(BMSInfoData* const& info) {
			if (info == nullptr || info->mFilePath.size() == 0) {
				LOG("Invalid BMSInfoData format");
				return;
			}
			clock_t s;
			if (info != mCurData.mInfo) {
				s = clock();
				mCurData.Reset(info, true);
				if (!mDecryptor.Build(true)) {
					LOG("parse bms failed : " + Utility::WideToUTF8(info->mFilePath));
					return;
				}
				LOG("bms data build time(ms) : " << clock() - s);
				mCurData.mReady = true;
			}

			LOG("Play data : " + Utility::WideToUTF8(mCurData.mInfo->mFilePath));
			s = clock();
			mThread.Play();
			LOG("mThread.Play time(ms) : " << clock() - s);
		}

		inline bool IsPlayingMusic() {
			return mThread.IsPlaying();
		}

		inline void TerminateMusic() {
			mThread.ForceEnd();
		}

		// ----- get, set function -----

		/// <summary> return all list of bms folder name </summary>
		const std::vector<std::string> GetFolderList() {
			return mPathTree.GetFolderList();
		}

		/// <summary> return proper list of bms music folder </summary>
		const std::vector<BMSNode>& GetMusicList(uint16_t index) {
			return mPathTree.GetMusicList(index);
		}

	private:
		///<summary> The class that stores all data of a single <see cref="bms::BMSInfoData"/> object </summary>
		BMSData mCurData;
		///<summary> The class that hat performs file interpretation and stores it in <see cref="bms::BMSData"/> object </summary>
		BMSDecryptor mDecryptor;
		///<summary> The class that manages the preview before playing the music </summary>
		PlayThread mThread;

		///<summary> The class to store and manage bms list and folder path </summary>
		BMSTree mPathTree;

		/// <summary> call <see cref="bms::BMSTree::Load()"/> function </summary>
		void Load() {
			clock_t s = clock();
			mPathTree.Load();
			LOG("mPathTree load time(ms) : " << clock() - s);
		}

		/// <summary> call <see cref="bms::BMSTree::Save()"/> function </summary>
		void Save() {
			clock_t s = clock();
			mPathTree.Save();
			LOG("mPathTree save time(ms) : " << clock() - s);
		}
	};
}