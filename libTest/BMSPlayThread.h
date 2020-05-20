#pragma once

#include "FMODWrapper.h"

#include <chrono>
#include <condition_variable>
#include <thread>
#include <future>
#include <unordered_set>
#include <array>

using namespace std::chrono_literals;	// for using std::this_thread::sleep_for() function
namespace bms {
	/// <summary>
	/// Frequency of refresh time checks
	/// </summary>
	constexpr auto FRAMERATE = 500;
	/// <summary>
	/// Maximum allowable error time if the loop misses the correct sound output time due to low frame. 
	/// Music with time outside that range is not output.
	/// == 2 * unit correction value / current framerate -> current : 4 milliseconds
	/// </summary>
	constexpr int MAX_ERROR_RANGE = 2000000 / FRAMERATE;
	/// <summary>
	/// Minimum playable time that can be guaranteed when loading music asynchronously. unit = microseconds
	/// After the loop starts, it guarantees playback until at least this time, even if no music is loaded. = By this time, the music is loaded synchronously.
	/// </summary>
	constexpr int ASYNC_READY_TIME = 5000000;
	/// <summary>
	/// The number of threads to use when generating the sound. It must not be lower than the minimum value of 1.
	/// caution : Since there are 2 lists, the thread is created twice. ex) value = 1 -> the number of thread = 2
	/// TODO : use std::thread::hardware_concurrency()
	/// </summary>
	constexpr int THREAD_NUM_FOR_LOADING = 2;

	/// <summary>
	/// A class that manages threads for playing music
	/// Only a maximum of one thread can be created.
	/// </summary>
	class PlayThread {
	public:
		PlayThread(BMSData& data) : mData(data), mStop(true) {
			// initialize FMOD library
			mFMOD.Init();
		} 
		~PlayThread() {
			std::cout << "PlayThread destructor" << std::endl;
			ForceEnd();
		}

		inline bool IsPlaying() {
			return !mStop && mPlayThread.joinable();
		}

		/// <summary>
		/// Ask the wrapper to generate the sounds in <paramref name="folderPath"/> and put it in <see cref="mDicSound"/> dictionary.
		/// </summary>
		void CreateSounds(const std::string& folderPath) {
			clock_t s = clock();

			// terminate prevthread if it activated
			ForceEndLoadingThread();

			// Up to a certain time, music files are loaded synchronously to ensure playback.
			std::unordered_set<int> syncSounds;
			int bgmCount = 0, noteCount = 0;
			mLoadingChecker.fill({});
			while (bgmCount < mMaxBgmCount && mData.mListBgm[bgmCount].mTime < ASYNC_READY_TIME)
				syncSounds.insert(mData.mListBgm[bgmCount++].mKey);
			while (noteCount < mMaxNoteCount && mData.mListPlayerNote[noteCount].mTime < ASYNC_READY_TIME)
				syncSounds.insert(mData.mListPlayerNote[noteCount++].mKey);

			for (int key : syncSounds) {
				std::string dicVal = mData.mListWavName[key];
				if (dicVal == "") continue;
				mFMOD.CreateSound(folderPath + dicVal, key);
				mLoadingChecker[key] = true;
			}
			LOG("FMOD sync sound create time(ms) : " << clock() - s)

			//LOG("mFuture init value : " << mFuture[0].valid())
			mLoadingController = true;
			for (int i = 0; i < THREAD_NUM_FOR_LOADING; ++i) {
				mFuture[i * 2] = std::async(std::launch::async, &PlayThread::AsyncSoundLoad, this, folderPath, true, bgmCount + i);
				mFuture[i * 2 + 1] = std::async(std::launch::async, &PlayThread::AsyncSoundLoad, this, folderPath, false, noteCount + i);
			}
			//LOG("mFuture deferred value : " << mFuture[0].valid())

			/*for (std::pair<int, std::string> element : dic) {
				if (syncSounds.count(element.first) == 0) {
					mFMOD.CreateSoundAsync(folderPath + element.second, element.first);
				}
			}*/
		}

		/// <summary>
		/// Function that terminates the loading thread immediately
		/// </summary>
		void ForceEndLoadingThread() {
			if (!mLoadingController) {
				return;
			}

			mLoadingController = false;
			for (int i = 0; i < THREAD_NUM_FOR_LOADING * 2; ++i) {
				try {
					mFuture[i].get();
				} catch (const std::exception& e) {
					std::cout << "future.get() exception : " << e.what() << std::endl;
				}
			}
		}

		/// <summary>
		/// function to create a thread and play music by reading <see cref="mData"/> of <see cref="BMS::BMSData"/>
		/// </summary>
		void Play() {
			// terminate if the thread is alive
			ForceEnd();

			mNoteIndex = 0;
			mBgmIndex = 0;
			if (!mFMOD.IsInitialized()) {
				LOG("FMOD system initialize failed");
				return;
			}

			clock_t s = clock();
			// initialization. preloading sound files
			std::wstring filePath = mData.mInfo->mFilePath;
			std::string utfPath = Utility::WideToUTF8(filePath.substr(0, filePath.find_last_of(L'/'))) + '/';
			mMaxBgmCount = static_cast<int>(mData.mListBgm.size());
			mMaxNoteCount = static_cast<int>(mData.mListPlayerNote.size());
			CreateSounds(utfPath);

			LOG("FMOD sound create time(ms) : " << clock() - s)

			mDuration = std::chrono::microseconds(mData.mInfo->mTotalTime + 500000ll);
			// music start
			mStop = false;
			// reference : https://stackoverflow.com/questions/35897617/c-loop-with-fixed-delta-time-on-a-background-thread
			// if you use member function to thread -> mThread = std::thread(&PlayThread::func, this);
			mPlayThread = std::thread([&]() {
				using delta = std::chrono::duration<std::int64_t, std::ratio<1, FRAMERATE>>;
				auto next = std::chrono::steady_clock::now() + delta{1};
				std::unique_lock<std::mutex> lock(mMutex);
				// originally : framerate time , modified : accumulated time
				//auto prev = std::chrono::steady_clock::now();
				auto start = std::chrono::steady_clock::now();
				while (!mStop) {
					mMutex.unlock();
					// Do stuff
					auto now = std::chrono::steady_clock::now();
					std::chrono::microseconds timeDelta =
						std::chrono::duration_cast<std::chrono::microseconds>(now - start);

					/*if (mDuration < timeDelta) {
						ForceEnd();
					}*/

					Update(timeDelta);

					//prev = now;
					//std::cout << "working: actual delta = " << timeDelta.count() << "¥ìs\n";
					// Wait for the next frame
					mMutex.lock();
					mConditionVar.wait_until(lock, next, [] {return false; });
					next += delta{1};
				}
			});

			// wait for the music to finish playing
			//std::this_thread::sleep_for();	// + 0.5sec
			//ForceEnd();
		}

		/// <summary>
		/// function called every 1 / <see cref="FRAMERATE"/> seconds within the thread
		/// </summary>
		/// <param name="delta"> The time accumulated to call this function in all cycle. </param>
		void Update(std::chrono::microseconds cDelta) {
			long long deltaVal = cDelta.count();
			long long minTime = deltaVal - MAX_ERROR_RANGE;

			// original versiont
			//while (mBgmIndex < mMaxBgmIndex && mListBgm[mBgmIndex].mTime < deltaVal) {
			//	mFMOD.PlaySound(true, mListBgm[mBgmIndex].mKey);
			//	//std::cout << "bgm play : " << mBgmIndex << std::endl;
			//	mBgmIndex++;
			//}
			while (mBgmIndex < mMaxBgmCount) {
				Note& note = mData.mListBgm[mBgmIndex];
				if (note.mTime >= deltaVal) {
					break;
				}
				// music outside the error range is not played.
				if (note.mTime > minTime) {
					mFMOD.PlaySingleSound(note.mKey);
				}
				mBgmIndex++;
			}
			while (mNoteIndex < mMaxNoteCount) {
				PlayerNote& note = mData.mListPlayerNote[mNoteIndex];
				if (note.mTime >= deltaVal) {
					break;
				}
				// play note (landmine doesn't have own sound == mute)
				if (note.mType != NoteType::LANDMINE && note.mTime > minTime) {
					mFMOD.PlaySingleSound(note.mKey);
				}
				mNoteIndex++;
			}

			mFMOD.Update();
		}

		/// <summary>
		/// function to call when you want to terminate the thread
		/// </summary>
		void ForceEnd() {
			// terminate loading threads
			ForceEndLoadingThread();

			if (!IsPlaying()) {
				return;
			}

			// Unlock the mutex because it will be destroyed when you go out of the block.
			{
				std::lock_guard<std::mutex> lock(mMutex);
				mStop = true;
			}
			mFMOD.ReleaseAllSounds();
			mPlayThread.join();
		}
	private:
		bool mStop;
		std::condition_variable mConditionVar;
		std::mutex mMutex;
		std::thread mPlayThread;
		std::chrono::microseconds mDuration;	// max thread duration

		bool mLoadingController;				// check if loading thread is working. set value to false if you want to terminate loading thread
		std::array<bool, 1297> mLoadingChecker;	// An array that stores whether a sound file with its index as a key value has been loaded
		std::mutex mLoadingMutex;
		std::future<bool> mFuture[THREAD_NUM_FOR_LOADING * 2];

		BMSData& mData;
		int mNoteIndex;							// used for note list looping
		int mBgmIndex;							// used for bgm list looping

		int mMaxNoteCount;
		int mMaxBgmCount;

		FMODWrapper mFMOD;



		/// <summary>
		/// function that defines a task for loading sound files asynchronously. it is an argument to std::async object
		/// bIsBgm value is true when loop is BGM, false when loop is player note
		/// </summary>
		bool AsyncSoundLoad(const std::string& folderPath, bool bIsBgm, int startPoint) {
			int max = bIsBgm ? mMaxBgmCount : mMaxNoteCount;
			while (startPoint < max && mLoadingController) {
				int key = bIsBgm ? mData.mListBgm[startPoint].mKey : mData.mListPlayerNote[startPoint].mKey;
				std::string dicVal = mData.mListWavName[key];
				mLoadingMutex.lock();
				// already created or no sound object -> skip
				if (mLoadingChecker[key] || dicVal == "") {
					startPoint += THREAD_NUM_FOR_LOADING;
					mLoadingMutex.unlock();
				} else {
					mLoadingChecker[key] = true;
					mLoadingMutex.unlock();
					mFMOD.CreateSound(folderPath + dicVal, key);
					startPoint += THREAD_NUM_FOR_LOADING;
				}
			}

			printf("terminate %s thread \n", bIsBgm ? "BGM" : "Note");
			return true;
		}
	};
}