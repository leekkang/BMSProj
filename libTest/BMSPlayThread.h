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
	/// </summary>
	constexpr int THREAD_NUM_FOR_LOADING = 2;

	/// <summary>
	/// A class that manages threads for playing music
	/// Only a maximum of one thread can be created.
	/// </summary>
	class PlayThread {
	public:
		PlayThread() : mStop(true) {
			// initialize FMOD library
			mFMOD.Init();
		} 
		~PlayThread() {
			std::cout << "PlayThread destructor" << std::endl;
			ForceEnd();
			mPrevFolderPath.clear();
		}

		inline bool IsPlaying() {
			return !mStop && mPlayThread.joinable();
		}

		/// <summary>
		/// Ask the wrapper to generate the sounds in <paramref name="folderPath"/> and put it in <see cref="mDicSound"/> dictionary.
		/// </summary>
		void CreateSounds(const std::string& folderPath, const std::unordered_map<int, std::string>& dic) {
			clock_t s = clock();

			// terminate prevthread if it activated
			if (mLoadingController) {
				ForceEndLoadingThread();
			}

			// Up to a certain time, music files are loaded synchronously to ensure playback.
			std::unordered_set<int> syncSounds;
			int bgmCount = 0, noteCount = 0;
			mLoadingChecker.fill({});
			while (mListBgm[bgmCount].mTime < ASYNC_READY_TIME) syncSounds.insert(mListBgm[bgmCount++].mKey);
			while (mListNote[noteCount].mTime < ASYNC_READY_TIME) syncSounds.insert(mListNote[noteCount++].mKey);
			for (int key : syncSounds) {
				auto value = (dic.find(key))->second;
				mFMOD.CreateSound(folderPath + value, key);
				mLoadingChecker[key] = true;
			}
			LOG("FMOD sync sound create time(ms) : " << clock() - s)

			// async load lambda expression. it is an argument to std::async object
			/// bIsBgm value is true when loop is BGM, false when loop is player note
			auto asyncLoad = [&](bool bIsBgm, int startPoint) -> bool{
				int max = bIsBgm ? mMaxBgmIndex : mMaxNoteIndex;
				while (startPoint < max && mLoadingController) {
					int key = bIsBgm ? mListBgm[startPoint].mKey : mListNote[startPoint].mKey;
					auto dicVal = mDicWave.find(key);
					mLoadingMutex.lock();
					// already created or no sound object -> skip
					if (mLoadingChecker[key] || dicVal == mDicWave.end()) {
						startPoint += THREAD_NUM_FOR_LOADING;
						mLoadingMutex.unlock();
					} else {
						mLoadingChecker[key] = true;
						mLoadingMutex.unlock();
						mFMOD.CreateSound(folderPath + dicVal->second, key);
						//mFMOD.CreateSoundAsync(folderPath + dicVal->second, key);
						//printf("create %s sound : %d, %d, %s\n", bIsBgm ? "BGM" : "Note", startPoint, key, (dicVal->second).c_str());
						startPoint += THREAD_NUM_FOR_LOADING;
					}
				}

				printf("terminate %s thread \n", bIsBgm ? "BGM" : "Note");
				return true;
			};

			LOG("mFuture init value : " << mFuture[0].valid())
			mLoadingController = true;
			for (int i = 0; i < THREAD_NUM_FOR_LOADING; ++i) {
				mFuture[i * 2] = std::async(std::launch::async, asyncLoad, true, bgmCount + i);
				mFuture[i * 2 + 1] = std::async(std::launch::async, asyncLoad, false, noteCount + i);
			}
			LOG("mFuture deferred value : " << mFuture[0].valid())

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
			mLoadingController = false;

			for (int i = 0; i < THREAD_NUM_FOR_LOADING * 2; ++i) {
				mFuture[i].get();
			}
		}

		/// <summary>
		/// function to create a thread and play music by reading <paramref name="data"/> of <see cref="BMS::BMSData"/>
		/// </summary>
		void Play(const std::string& folderPath, const BMSData& data) {
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
			if (mPrevFolderPath != folderPath) {
				mListBgm = data.mListBgm;
				mListNote = data.mListPlayerNote;
				mMaxBgmIndex = mListBgm.size(); 
				mMaxNoteIndex = mListNote.size();
				mDicWave = data.mDicWav;
				LOG("copy vector time(ms) : " << clock() - s)

				s = clock();
				CreateSounds(folderPath, data.mDicWav);
				mPrevFolderPath = folderPath;
			} else {
				printf("all sounds already created\n");
			}

			LOG("FMOD sound create time(ms) : " << clock() - s)

			mDuration = std::chrono::microseconds(data.mTotalTime + 500000ll);
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
					//std::cout << "working: actual delta = " << timeDelta.count() << "��s\n";
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
			while (mBgmIndex < mMaxBgmIndex && mListBgm[mBgmIndex].mTime < deltaVal) {
				// music outside the error range is not played.
				if (mListBgm[mBgmIndex].mTime > minTime) {
					mFMOD.PlaySound(mListBgm[mBgmIndex].mKey);
				}
				//std::cout << "bgm play : " << mBgmIndex << std::endl;
				mBgmIndex++;
			}
			while (mNoteIndex < mMaxNoteIndex && mListNote[mNoteIndex].mTime < deltaVal) {
				// play note (landmine doesn't have own sound == mute)
				if (mListNote[mNoteIndex].mType != NoteType::LANDMINE && mListNote[mNoteIndex].mTime > minTime) {
					mFMOD.PlaySound(mListNote[mNoteIndex].mKey);
				}
				//std::cout << "note play : " << mNoteIndex << std::endl;
				mNoteIndex++;
			}

			mFMOD.Update();
		}

		/// <summary>
		/// function to call when you want to terminate the thread
		/// </summary>
		void ForceEnd() {
			if (!IsPlaying()) {
				return;
			}

			// Unlock the mutex because it will be destroyed when you go out of the block.
			{
				std::lock_guard<std::mutex> lock(mMutex);
				mStop = true;
			}
			mPlayThread.join();
		}
	private:
		bool mStop;
		std::condition_variable mConditionVar;
		std::mutex mMutex;
		std::thread mPlayThread;
		std::chrono::microseconds mDuration;	// max thread duration

		std::string mPrevFolderPath;			// Folder path of previously loaded sound

		bool mLoadingController;				// check if loading thread is working. set value to false if you want to terminate loading thread
		std::array<bool, 1297> mLoadingChecker;	// An array that stores whether a sound file with its index as a key value has been loaded
		std::mutex mLoadingMutex;
		std::future<bool> mFuture[THREAD_NUM_FOR_LOADING * 2];

		int mNoteIndex;							// used for note list looping
		int mBgmIndex;							// used for bgm list looping

		int mMaxNoteIndex;
		int mMaxBgmIndex;
		std::vector<Note> mListBgm;
		std::vector<PlayerNote> mListNote;
		std::unordered_map<int, std::string> mDicWave;

		FMODWrapper mFMOD;
	};
}