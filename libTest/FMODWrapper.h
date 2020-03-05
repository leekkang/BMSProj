#pragma once

#include "fmod.hpp"
/// <summary>
	/// A wrapper class that wraps the FMOD system in the necessary shape.
	/// </summary>
class FMODWrapper {
public:
	FMODWrapper() = default;
	~FMODWrapper() {
		if (mInitialized) {
			ReleaseAllSounds();
			system->release();
			mInitialized = false;
		}
	};

	DISALLOW_COPY_AND_ASSIGN(FMODWrapper)
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
		result = system->init(1024, FMOD_INIT_NORMAL, extradriverdata);
		if (IsJobFailed("system->getVersion failed")) return false;

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

		// Uncomment to see how many songs are currently playing on this channel.
		//int channelsplaying = 0;
		//system->getChannelsPlaying(&channelsplaying, NULL);
	}

	/// <summary>
	/// create <see cref="FMOD::Sound"/> files in <paramref name="folderPath"/> and put it in <see cref="mDicSound"/> dictionary.
	/// </summary>
	void CreateSounds(const std::string& folderPath, const std::unordered_map<int, std::string>& dic) {
		if (!mInitialized) {
			printf("The system did not initialize\n");
			return;
		}

		if (mPrevFolderPath == folderPath) {
			printf("all sounds already created\n");
			return;
		}

		for (std::pair<int, std::string> element : dic) {
			FMOD::Sound* sound;
			result = system->createSound((folderPath + element.second).c_str(), FMOD_LOOP_OFF, 0, &sound);
			if (IsJobFailed("failed to create sound : " + element.second)) {
				exit(-1);
			}
			if (mDicSound.count(element.first) != 0) {
				mDicSound[element.first]->release();
			}
			mDicSound[element.first] = sound;
		}

		mPrevFolderPath = folderPath;
	}

	/// <summary>
	/// release all <see cref="FMOD::Sound"/> file memories in <paramref name="folderPath"/> and clear dictionary.
	/// </summary>
	void ReleaseAllSounds() {
		for (std::pair<int, FMOD::Sound*> element : mDicSound) {
			element.second->release();
		}
		mDicSound.clear();
		mPrevFolderPath.clear();
	}

	/// <summary>
	/// play sound file in <paramref name="folderPath"/> dictionary.
	/// </summary>
	/// <>
	void PlaySound(int key) {
		FMOD::Sound* sound = mDicSound[key];
		result = system->playSound(sound, 0, false, &channel);
		IsJobFailed("PlaySound failed : " + key);
	}

private:
	bool mInitialized;
	FMOD_RESULT result;

	FMOD::System* system;
	FMOD::Channel* channel = 0;
	unsigned int version;
	void* extradriverdata = 0;

	std::string mPrevFolderPath;
	std::unordered_map<int, FMOD::Sound*> mDicSound;

	/// <summary> check if last processed job was successful. if failed, write <paramref name="output"/> to console </summary>
	bool IsJobFailed(const std::string& output = "") {
		bool bFailed = result != FMOD_OK;
		if (bFailed && output.size() > 0) {
			printf("%s\n", output.c_str());
		}

		return bFailed;
	}
};