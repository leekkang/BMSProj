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

		BMSAdapter() : mDecryptor(mCurData), mPathTree(mDecryptor){
			Load();
		};
		~BMSAdapter() {
			Save();
		};
		DISALLOW_COPY_AND_ASSIGN(BMSAdapter)
		BMSAdapter(BMSAdapter&&) noexcept = default;
		BMSAdapter& operator=(BMSAdapter&&) noexcept = default;

		// ----- user access function -----

		/// <summary> call <see cref="bms::BMSTree::Load()"/> function </summary>
		void Load() {
			mPathTree.Load(mDecryptor);
		}

		/// <summary> call <see cref="bms::BMSTree::Save()"/> function </summary>
		void Save() {
			mPathTree.Save();
		}

		/// <summary>
		/// make <see cref="BMS::BMSData"/> object by reading the bms file in <paramref name="path"/>
		/// </summary>
		/// <returns> return true if a <see cref="bms::BMSData"/> object is correctly build </returns>
		bool Make(const std::string& path);

		void Play(int index);

		inline bool IsPlayingMusic() {
			return mThread.IsPlaying();
		}

		inline void TerminateMusic() {
			if (mThread.IsPlaying()) {
				mThread.ForceEnd();
			}
		}

		// ----- get, set function -----
	private:
		///<summary> The class that stores all data of a single <see cref="bms::BMSInfoData"/> object </summary>
		BMSData mCurData;
		///<summary> The class that hat performs file interpretation and stores it in <see cref="bms::BMSData"/> object </summary>
		BMSDecryptor mDecryptor;
		///<summary> The class that manages the preview before playing the music </summary>
		PlayThread mThread;

		///<summary> The class to store and manage bms list and folder path </summary>
		BMSTree mPathTree;

		///<summary> List that stores the completed bms info data instance </summary>
		std::vector<BMSInfoData> mListData;
		///<summary> Folder path where all bms related files are stored </summary>
		std::vector<std::string> mListFolderPath;
	};
}