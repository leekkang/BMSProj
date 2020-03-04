#pragma once

#include "FMODWrapper.h"

#include <chrono>
#include <condition_variable>
#include <thread>

using namespace std::chrono_literals;	// for using std::this_thread::sleep_for() function
namespace bms {
	constexpr auto FRAMERATE = 120;

	/// <summary>
	/// A class that manages threads for playing music
	/// Only a maximum of one thread can be created.
	/// </summary>
	class PlayThread {
	public:
		PlayThread() : mStop(true) {
			// initialize FMOD library
		}
		~PlayThread() {
			std::cout << "PlayThread destructor" << std::endl;
			ForceEnd();
		}

		inline bool IsPlaying() {
			return !mStop && mThread.joinable();
		}

		/// <summary>
		/// function to create a thread and play music by reading <paramref name="data"/> of <see cref="BMS::BMSData"/>
		/// </summary>
		void Play(const std::string& folderPath, const BMSData& data) {
			// terminate if the thread is alive
			ForceEnd();

			// initialization. preloading sound files
			clock_t s = clock();
			mNoteIndex = 0;
			mBgmIndex = 0;
			mListBgm = data.mListBgm;
			mListNote = data.mListPlayerNote;
			mMaxBgmIndex = mListBgm.size();
			mMaxNoteIndex = mListNote.size();
			LOG("copy vector time(ms) : " << clock() - s)
			s = clock();
			if (!mFMOD.Init()) {
				LOG("FMOD system initialize failed");
				return;
			}
			mFMOD.CreateSounds(folderPath, data.mDicWav);
			LOG("FMOD sound create time(ms) : " << clock() - s)

			// music start
			mStop = false;
			// reference : https://stackoverflow.com/questions/35897617/c-loop-with-fixed-delta-time-on-a-background-thread
			// if you use member function to thread -> mThread = std::thread(&PlayThread::func, this);
			mThread = std::thread([&]() {
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
			std::this_thread::sleep_for(std::chrono::microseconds(data.mTotalTime + 500000ll));	// + 0.5sec
			ForceEnd();
		}

		/// <summary>
		/// function called every 1 / <see cref="FRAMERATE"/> seconds within the thread
		/// <param name="delta"> The time accumulated to call this function in all cycle. </param>
		/// </summary>
		void Update(std::chrono::microseconds cDelta) {
			long long deltaVal = cDelta.count();

			mFMOD.Update();

			while (mBgmIndex < mMaxBgmIndex && mListBgm[mBgmIndex].mTime < deltaVal) {
				// play bgm
				mFMOD.PlaySound(mListBgm[mBgmIndex].mKey);
				//std::cout << "bgm play : " << mBgmIndex << std::endl;
				mBgmIndex++;
			}
			while (mNoteIndex < mMaxNoteIndex && mListNote[mNoteIndex].mTime < deltaVal) {
				// play note
				mFMOD.PlaySound(mListNote[mNoteIndex].mKey);
				//std::cout << "note play : " << mNoteIndex << std::endl;
				mNoteIndex++;
			}
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
			mThread.join();
		}
	private:
		bool mStop;
		std::condition_variable mConditionVar;
		std::mutex mMutex;
		std::thread mThread;

		int mNoteIndex;
		int mBgmIndex;

		int mMaxNoteIndex;
		int mMaxBgmIndex;
		std::vector<Note> mListBgm;
		std::vector<PlayerNote> mListNote;

		FMODWrapper mFMOD;
		//void loopFunction() {}
	};
}