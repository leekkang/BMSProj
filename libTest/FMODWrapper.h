#pragma once

#include "fmod.hpp"
#include <iostream>
#include <unordered_map>
#include <mutex>

namespace bms {
	/// <summary>
	/// A wrapper class that wraps the FMOD system in the necessary shape.
	/// information : https://documentation.help/FMOD-API/introduction.html
	///				  https://www.fmod.com/resources/documentation-api?version=2.0&page=core-api-system.html
	/// </summary>
	class FMODWrapper {
	public: 
		int sLoadedMusicNum;		// The number of music currently loaded

		FMODWrapper() = default;
		~FMODWrapper() {
			if (mInitialized) {
				ReleaseAllSounds();
				system->release();
				mInitialized = false;
			}
		};

		FMODWrapper(const FMODWrapper&) = delete;
		FMODWrapper& operator=(const FMODWrapper&) = delete;
		FMODWrapper(FMODWrapper&& others) noexcept = default;
		FMODWrapper& operator=(FMODWrapper&&) noexcept = default;

		inline bool IsInitialized() {
			return mInitialized;
		}

		/// <summary> Function to initialize local variables needed for FMOD system </summary>
		bool Init() {
			mInitialized = false;
			result = FMOD::System_Create(&system);
			if (IsJobFailed("System_Create failed")) return false;
			result = system->getVersion(&version);	// isn't it necessary?
			if (IsJobFailed("system->getVersion failed")) return false;

			// set the maximum number of software mixed channels possible.
			// must be called before System::init
			// reference : https://documentation.help/FMOD-API/FMOD_System_SetSoftwareChannels.html
			system->setSoftwareChannels(128);
			if (IsJobFailed("system->setSoftwareChannels failed")) return false;

			result = system->init(1024, FMOD_INIT_NORMAL, extradriverdata);
			if (IsJobFailed("system->init failed")) return false;

			sLoadedMusicNum = 0;
			mInitialized = true;

			return true;
		}

		/// <summary> Function to periodically update the FMOD system </summary>
		void Update() {
			if (!mInitialized) {
				return;
			}

			result = system->update();
			if (IsJobFailed()) return;

			//printf("%d\n", sLoadedMusicNum);

			// Uncomment to see how many songs are currently playing on this channel.
			/*int channelsplaying = 0;
			system->getChannelsPlaying(&channelsplaying);
			printf("%d\n", channelsplaying);*/
		}

		/// <summary>
		/// create <see cref="FMOD::Sound"/> files and put it in the <paramref name="key"/> value of <see cref="mDicSound"/> dictionary.
		/// </summary>
		void CreateSound(const std::string& filePath, int key) {
			// sound option reference : https://documentation.help/FMOD-API/FMOD_MODE.html
			FMOD::Sound* sound;
			result = system->createSound(filePath.c_str(), FMOD_LOOP_OFF | FMOD_LOWMEM, 0, &sound);

			std::lock_guard<std::mutex> guard{mMutex};
			// TODO : replace exit to logging form that can easily see.
			if (IsJobFailed("failed to create sound : " + filePath)) {
				std::wstring path = Utility::UTF8ToWide(filePath);
				struct stat buffer;
				bool isExist = _wstat(path.data(), (struct _stat64i32*)&buffer) == 0;
				exit(-1);
				return;
			}

			if (mDicSound.count(key) != 0) {
				mDicSound[key]->release();
				if (!IsJobFailed("FMOD::Sound->release failed")) {
					sLoadedMusicNum--;
				}
			}

			mDicSound[key] = sound;
			sLoadedMusicNum++;
		}

		/// <summary>
		/// create <see cref="FMOD::Sound"/> files and put it in the <paramref name="key"/> value of <see cref="mDicSound"/> dictionary.
		/// </summary>
		void CreateSoundAsync(const std::string& filePath, int key) {
			// sound option reference : https://documentation.help/FMOD-API/FMOD_MODE.html
			FMOD::Sound* sound;
			result = system->createSound(filePath.c_str(), FMOD_NONBLOCKING, 0, &sound);

			std::lock_guard<std::mutex> guard{mMutex};
			// TODO : replace exit to logging form that can easily see.
			if (IsJobFailed("failed to create sound : " + filePath)) {
				exit(-1);
				return;
			}

			if (mDicSound.count(key) != 0) {
				mDicSound[key]->release();
				if (!IsJobFailed("FMOD::Sound->release failed")) {
					sLoadedMusicNum--;
				}
			}

			mDicSound[key] = sound;
			sLoadedMusicNum++;
		}

		/// <summary>
		/// release all <see cref="FMOD::Sound"/> file memories in <paramref name="folderPath"/> and clear dictionary.
		/// </summary>
		void ReleaseAllSounds() {
			for (std::pair<int, FMOD::Sound*> element : mDicSound) {
				element.second->release();
				if (!IsJobFailed("FMOD::Sound->release failed")) {
					sLoadedMusicNum--;
				}
			}
			mDicSound.clear();
		}

		/// <summary>
		/// play sound file in <paramref name="folderPath"/> dictionary.
		/// </summary>
		inline void PlaySingleSound(int key) {
			FMOD::Sound* sound = mDicSound[key];
			result = system->playSound(sound, 0, false, &channel);
			IsJobFailed("PlaySound failed : " + key);
		}

	private:
		bool mInitialized;
		FMOD_RESULT result;

		FMOD::System* system;
		FMOD::Channel* channel;
		unsigned int version;
		void* extradriverdata = 0;

		std::mutex mMutex;
		std::unordered_map<int, FMOD::Sound*, Utility::Bypass> mDicSound;

		/// <summary> check if last processed job was successful. if failed, write <paramref name="output"/> to console </summary>
		bool IsJobFailed(const std::string& output = "") {
			bool bFailed = result != FMOD_OK;
			if (bFailed && output.size() > 0) {
				printf("%s\n", output.c_str());
			}

			return bFailed;
		}
	};
}