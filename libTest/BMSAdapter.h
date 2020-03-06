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

		bool MakeFolder(const std::string& folderPath);

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
		///<summary> Folder path where all bms related files are stored </summary>
		std::string mFolderPath;

		///<summary> The class that manages the preview before playing the music. </summary>
		PlayThread mThread;

		///<summary> List that stores the completed bms data instance </summary>
		std::vector<BMSData> mListData;
	};
}