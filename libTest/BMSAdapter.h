#pragma once

#include "BMSDecryptor.h"
#include "BMSPlayThread.h"

namespace bms {
	/// <summary>
	/// A class that manages BMSData and 
	/// Only a maximum of one thread can be created.
	/// </summary>
	class BMSAdapter {
	public:
		// ----- constructor, operator overloading -----

		BMSAdapter() = default;
		~BMSAdapter() {
			//for (BMSData& data : mListData)
		};
		DISALLOW_COPY_AND_ASSIGN(BMSAdapter)
		BMSAdapter(BMSAdapter&&) noexcept = default;
		BMSAdapter& operator=(BMSAdapter&&) noexcept = default;

		// ----- user access function -----

		/// <summary>
		/// make <see cref="BMS::BMSData"/> object by reading the bms file in <paramref name="path"/>
		/// </summary>
		/// <returns> return true if a <see cref="BMS::BMSData"/> object is correctly build </returns>
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
		///<summary> The class that manages the preview before playing the music </summary>
		PlayThread mThread;

		///<summary> List that stores the completed bms info data instance </summary>
		std::vector<BMSInfoData> mListData;
		///<summary> Folder path where all bms related files are stored </summary>
		std::vector<std::string> mListFolderPath;
	};
}