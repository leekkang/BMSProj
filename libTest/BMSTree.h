#pragma once

#include "dirent.h"
#include <functional>

namespace bms {
	constexpr auto ROOT_PATH = L"StreamingAssets";
	//constexpr auto ROOT_PATH = L"E:/∑Á∫Í¿’";
	constexpr auto CACHE_FILE_NAME = "test.bin";

	/// <summary>
	/// A structure that stores a group of bms files for one song (has variable pattern)
	/// </summary>
	struct BMSNode {
		std::wstring mFolderName;
		std::vector<BMSInfoData*> mListData;

		/// <summary> used to reduce the string comparison overhead by saving the folder that has been checked when performing file system search. </summary>
		bool mConfirmed;

		BMSNode(const std::wstring& name, const std::vector<BMSInfoData*>& list) : mFolderName(name), mListData(list) {};
	};

	/// <summary>
	/// A structure that wrapping <see cref="dirent.h"/> functions
	/// </summary>
	class DirLoop {
		DIR* dir;
	public:
		DirLoop(const std::wstring& path) {
			dir = OpenDir(path);
		}
		~DirLoop() {
			CloseDir(dir);
		}

		// reference : https://docs.microsoft.com/ko-kr/cpp/c-runtime-library/filename-search-functions?view=vs-2019
		/// <summary> check this attribute mean subdirectory </summary>
		inline bool IsDirectory() {
			return dir->info.attrib & 0x10;	 // _A_SUBDIR : 0x10, normal file(_A_ARCH) : 0x20
		}

		inline wchar_t* Read() {
			if (!dir) {
				return nullptr;
			}
			wchar_t* result;
			// 1. goto implementation
		FOLDER_SYMBOL:
			if ((result = ReadDir(dir)) != nullptr) {
				if (wcscmp(result, CURRENT_FOLDER_SYM) == 0 || wcscmp(result, PREV_FOLDER_SYM) == 0) {
					goto FOLDER_SYMBOL;
				}
			}
			// 2. while implementation
			/*while ((result = ReadDir(dir)) != nullptr) {
				if (wcscmp(result, CURRENT_FOLDER_SYM) == 0 || wcscmp(result, PREV_FOLDER_SYM) == 0) {
					continue;
				}
			}*/

			return result;
		}
	};

	/// <summary>
	/// A class that stores a group of bms folders and provides convenience functions
	/// </summary>
	class BMSTree {
		/// <summary> specify folder type </summary>
		enum class FolderType : uint8_t {
			MUSIC,		// music folder include bms files
			PARENT,		// parent folder include one or more bms music folder
			NONE,		// neither a music folder, nor a parent folder
		};

	public:
		BMSTree(BMSDecryptor& decryptor) : mDecryptor(decryptor), mMusicSortOpt(SortOption::PATH_ASC), 
																  mPatternSortOpt(SortOption::LEVEL_ASC) {
			mMusicSortFunc = GetMusicSortFunc(mMusicSortOpt);
			mPatternSortFunc = GetPatternSortFunc(mPatternSortOpt);
		};
		DISALLOW_COPY_AND_ASSIGN(BMSTree)
		BMSTree(BMSTree&&) noexcept = default;
		BMSTree& operator=(BMSTree&&) noexcept = default;
		~BMSTree() {
			for (const auto& e : mDicBms) {
				for (const auto& node : e.second) {
					for (auto data : node.mListData) {
						delete data;
					}
				}
			}
		}

		/// <summary> return all list of bms folder name </summary>
		inline const std::vector<std::string> GetFolderList() {
			uint16_t size = static_cast<uint16_t>(mListFolder.size());
			std::vector<std::string> vec(size);
			for (int i = 0; i < size; ++i) {
				vec[i] = Utility::WideToUTF8(GetFileName(mListFolder[i].first));
			}
			return vec;
		}

		/// <summary> return proper list of bms music folder </summary>
		inline const std::vector<BMSNode>& GetMusicList(uint16_t index) {
			if (index >= mListFolder.size()) {
				throw std::out_of_range("mListFolder index is out of range");
			}
			std::pair<std::wstring, bool>& folder = mListFolder[index];
			// check update value
			if (!folder.second) {
				clock_t s = clock();
				SetMusicList(folder.first);
				folder.second = true;
				std::cout << "subdirectory load time(ms) : " << std::to_string(clock() - s) << '\n';
			}
			// if iter == mDicBms.end(), this is an error that should not happen because it is set once above.
			auto iter = mDicBms.find(mListFolder[index].first);
			return iter->second;
		}

		/// <summary> return path of bms pattern </summary>
		BMSInfoData* GetPattern(uint16_t folderIndex, uint16_t musicIndex, uint8_t patternIndex) {
			const std::vector<BMSNode>& music = GetMusicList(folderIndex);
			if (musicIndex >= music.size()) {
				return nullptr;
			}

			const std::vector<BMSInfoData*>& pattern = music[musicIndex].mListData;
			if (patternIndex >= pattern.size()) {
				return nullptr;
			}

			return pattern[patternIndex];
		}

		/// <summary> change the sorting option of all bms music folder lists to <paramref name="opt"/> </summary>
		void ChangeMusicSortOpt(SortOption opt) {
			mMusicSortOpt = opt;
			mMusicSortFunc = GetMusicSortFunc(mMusicSortOpt);
			for (auto& e : mDicBms) {
				auto& listMusic = e.second;
				std::sort(listMusic.begin(), listMusic.end(), mMusicSortFunc);
			}
		}
		/// <summary> change the sorting option of all music pattern lists to <paramref name="opt"/> </summary>
		void ChangePatternSortOpt(SortOption opt) {
			mPatternSortOpt = opt;
			mPatternSortFunc = GetPatternSortFunc(mPatternSortOpt);
			for (auto& e : mDicBms) {
				auto& vec = e.second;
				for (auto& v : vec) {
					auto& listPattern = v.mListData;
					std::sort(listPattern.begin(), listPattern.end(), mPatternSortFunc);
				}
			}
		}

		/// <summary> save all <see cref="mDicFolderName"/> elements to binary file </summary>
		void Save() {
			if (!mChangeSave) {
				std::cout << "nothing changed" << std::endl;
				return;
			}
			clock_t s = clock();
			std::ofstream os(CACHE_FILE_NAME, std::ios::binary);
			// save bms container
			for (const auto& e : mDicBms) {
				for (const auto& node : e.second) {
					std::string uPath = Utility::WideToUTF8(PathAppend(e.first, node.mFolderName));
					uint8_t size = static_cast<uint8_t>(node.mListData.size());
					WriteToBinary(os, uPath);
					WriteToBinary(os, size);
					for (auto& info : node.mListData) {
						os << *info;
					}
				}
			}
			os.close();
			std::cout << "BMSInfoData save time(ms) : " << std::to_string(clock() - s) << '\n';
		}

		/// <summary>
		/// check file paths and load <see cref="bms::BMSInfoData"/> list from cache file
		/// If the file does not exist or the path is changed, the object is created again to update the list.
		/// </summary>
		void Load() {
			mChangeSave = false;
			// 1. set parent folder info
			// 2. set folder index to 0 of list
			// 3. load cache data
			//  1) cache data exists
			//	  -> check first bms parent folder and modify parent folder list
			//  2) no cache data exists
			//	  -> check first bms parent folder and create parent folder list

			clock_t s = clock();
			// set folder map. used to check if a folder exists and to create a new folder list.
			{
				// make folder map(tmpFolder)
				std::unordered_map<std::wstring, uint16_t> tmpFolder;
				DirLoop loop(ROOT_PATH);
				wchar_t* name;
				bool bIncludeRoot = false;	// true if a root folder has bms music folder
				uint16_t index = 1;
				while (name = loop.Read()) {
					if (loop.IsDirectory()) {
						std::wstring path = PathAppend(ROOT_PATH, name);
						FolderType type = GetFolderType(path);
						if (type == FolderType::PARENT) {
							tmpFolder.emplace(path, index++);
						} else if (!bIncludeRoot && type == FolderType::MUSIC) {
							bIncludeRoot = true;
							tmpFolder.emplace(ROOT_PATH, 0);
						}
					}
				}
				// make folder list using map
				mListFolder.resize(tmpFolder.size());
				for (const auto& e : tmpFolder) {
					index = bIncludeRoot ? e.second : e.second - 1;	// no bms files in root folder
					mListFolder[index] = std::pair<std::wstring, bool>(e.first, false);
				}
			}
			std::cout << "parent folder check time(ms) : " << std::to_string(clock() - s) << '\n';

			// throw exception if no bms file in ROOT_PATH
			if (mListFolder.size() == 0) {
				throw std::runtime_error("no bms files in " + Utility::WideToAnsi(ROOT_PATH, std::locale()));
			}

			// load cache data
			s = clock();
			std::wstring indexedFolderName;
			std::ifstream is(CACHE_FILE_NAME, std::ios::binary);
			if (is.is_open()) {
				std::wstring wPath;
				uint8_t size;
				while (is.peek() != std::ifstream::traits_type::eof()) {
					wPath = Utility::UTF8ToWide(ReadFromBinary<std::string>(is));
					size = ReadFromBinary<uint8_t>(is);
					if (!IsExistFile(wPath)) {
						// folder is not found -> discard
						BMSInfoData temp;
						for (uint8_t i = 0; i < size; ++i) {
							is >> temp;
						}
						continue;
					}

					std::vector<BMSInfoData*> vec; vec.reserve(size);
					for (uint8_t i = 0; i < size; ++i) {
						BMSInfoData* temp = new BMSInfoData();
						is >> *temp;
						if (!IsExistFile(temp->mFilePath)) {
							delete temp;
						} else {
							vec.emplace_back(temp);
						}
					}
					AddMusic(wPath, std::move(vec));
				}
			}
			is.close();
			std::cout << "cache load time(ms) : " << std::to_string(clock() - s) << '\n';

			s = clock();
			// check first subdirectory and create added bms files or folder.
			SetMusicList(mListFolder[0].first);
			mListFolder[0].second = true;
			std::cout << "subdirectory load time(ms) : " << std::to_string(clock() - s) << '\n';
		}

	private:
		BMSDecryptor& mDecryptor;

		/// <summary> variable to check for changes when loading cache files </summary>
		bool mChangeSave;
		SortOption mMusicSortOpt;		// sorting option of bms music list
		SortOption mPatternSortOpt;		// sorting option of pattern list in single <see cref="bms::BMSNode"/>
		std::function<bool(const BMSNode&, const BMSNode&)> mMusicSortFunc;				// sort function of bms music list
		std::function<bool(BMSInfoData* const&, BMSInfoData* const&)> mPatternSortFunc;	// sort function of pattern list

		/// <summary>
		/// A list that indicate the parent folder path. Used for search bms data list.
		/// first : folder name, second : check to complete filesystem scan and already updated
		/// </summary>
		std::vector<std::pair<std::wstring, bool>> mListFolder;

		/// <summary>
		/// A dictionary with a folder path as the key and a music folder list as the value.
		/// It is used for music indexing, ui display, etc.
		/// hierarchy : parent folders -> music folders -> patterns
		/// </summary>
		std::unordered_map<std::wstring, std::vector<BMSNode>> mDicBms;

		/// <summary> simple path append for new wstring object </summary>
		inline std::wstring PathAppend(const std::wstring& p1, const std::wstring& p2) {
			std::wstring ws(p1);
			ws.push_back(L'/');
			return ws.append(p2);
		}
		/// <summary> simple filename get function of <paramref name="path"/> wstring </summary>
		inline std::wstring GetFileName(const std::wstring& path) {
			return &(path[path.find_last_of(L'/') + 1]);
		}
		/// <summary> simple parent folder path get function of <paramref name="p"/> wstring </summary>
		inline std::wstring GetDirectory(const std::wstring& path) {
			return path.substr(0, path.find_last_of(L'/'));
		}

		/// <summary> check that the <paramref name="path"/> is a valid file / folder path </summary>
		inline bool IsExistFile(const std::wstring& path) {
			struct stat buffer;
			return _wstat(path.data(), (struct _stat64i32*)&buffer) == 0;
		}

		/// <summary> check whether <param name="name"/> is bms file </summary>
		inline bool IsBmsFile(const wchar_t* name) {
			// bms file extension : .bms, .bme, .bml
			size_t len = wcslen(name);
			if (len < 4 || name[len - 4] != '.' || name[len - 3] != 'b' || name[len - 2] != 'm') {
				return false;
			}
			wchar_t w = name[len - 1];
			return w == 's' || w == 'e' || w == 'l';
		}
		/// <summary> check whether <param name="name"/> is sound file (extension : wav, ogg, mp3) </summary>
		inline bool IsSoundFile(const wchar_t* name) {
			size_t len = wcslen(name);
			if (len < 4 || name[len - 4] != '.') {
				return false;
			}
			wchar_t w = name[len - 3];
			if (w == 'w') {
				return name[len - 2] == 'a' && name[len - 1] == 'v';
			} else if (w == 'o') {
				return name[len - 2] == 'g' && name[len - 1] == 'g';
			} else if (w == 'm') {
				return name[len - 2] == 'p' && name[len - 1] == '3';
			}
			return false;
		}

		/// <summary> function to check what type of folder this <paramref name="path"/> folder is </summary>
		inline FolderType GetFolderType(const std::wstring& path) {
			// find depth 2 bms folder
			DirLoop loop(path);
			wchar_t* name;
			while (name = loop.Read()) {
				if (!loop.IsDirectory()) {
					// check this folder is bms folder itself. 
					// in this situation, parent folder of this folder becomes that i'm looking for.
					if (IsBmsFile(name)) {
						return FolderType::MUSIC;
					}
					continue;
				}

				DirLoop subloop(PathAppend(path, name));
				while (name = subloop.Read()) {
					if (!subloop.IsDirectory() && IsBmsFile(name)) {
						return FolderType::PARENT;
					}
				}
			}
			return FolderType::NONE;
		}

		/// <summary> add <see cref="bms::BMSNode"/> object into dictionary </summary>
		inline void AddMusic(const std::wstring& path, std::vector<BMSInfoData*>&& patterns) {
			std::wstring key = GetDirectory(path);
			if (mDicBms.count(key) == 0) {
				std::vector<BMSNode> vec;
				vec.emplace_back(GetFileName(path), std::move(patterns));
				mDicBms.emplace(key, std::move(vec));
			} else {
				std::vector<BMSNode>& vec = mDicBms[key];
				vec.emplace_back(GetFileName(path), std::move(patterns));
			}
		}

		/// <summary>
		/// find bms file and store in dictionary. if new pattern is found, create new <see cref="bms::BMSInfoData"/> object
		/// </summary>
		void SetMusicList(const std::wstring& folderPath) {
			std::vector<BMSNode> dummyList;
			auto iter = mDicBms.find(folderPath);
			std::vector<BMSNode>& musicList = iter == mDicBms.end() ? dummyList : iter->second;
			size_t initMusicNum = musicList.size();
			std::vector<std::wstring> patternPathList; patternPathList.reserve(32);

			auto findMusicNameIndex = [initMusicNum](const std::vector<BMSNode>& vec, const std::wstring& ws, bool* checker) {
				for (int i = 0; i < initMusicNum; ++i) {
					if (!checker[i] && vec[i].mFolderName == ws) {
						checker[i] = true;
						return i;
					}
				}
				return -1;
			};
			auto checkPatternExist = [](const std::vector<BMSInfoData*>& vec, const std::wstring& ws, bool* checker) {
				size_t len = vec.size();
				for (int i = 0; i < len; ++i) {
					if (!checker[i] && vec[i]->mFilePath == ws) {
						checker[i] = true;
						return true;
					}
				}
				return false;
			};

			bool* folderChecker = new bool[initMusicNum] {};	// used to reduce the string comparison overhead by saving the folder 
																// that has been checked when performing file system search.
			bool bIncMusicNum = false;							// variable to check if more than one music is added
			wchar_t* name;
			DirLoop loop(folderPath);
			while (name = loop.Read()) {
				if (!loop.IsDirectory()) {
					continue;
				} 

				// bms file check ready
				bool bCheckSoundExt = false;
				std::string extension;
				patternPathList.clear();

				// subloop to find bms files + confirm sound extension
				std::wstring subPath = PathAppend(folderPath, name);
				wchar_t* subName;
				DirLoop subloop(subPath);
				while (subName = subloop.Read()) {
					if (subloop.IsDirectory()) {
						continue;
					}

					// bms pattern check
					if (IsBmsFile(subName)) {
						patternPathList.emplace_back(PathAppend(subPath, subName));
					} else if (!bCheckSoundExt && IsSoundFile(subName)) {
						// sound file extension check
						std::wstring ext = &(subName[wcslen(subName) - 3]);
						extension.assign(ext.begin(), ext.end());
						bCheckSoundExt = true;
					}
				}

				// check whether this folder is bms music folder
				uint8_t musicCount = static_cast<uint8_t>(patternPathList.size());
				if (musicCount == 0) {
					continue;
				}

				// music folder is not found -> add music to dictionary
				int musicIndex = findMusicNameIndex(musicList, name, folderChecker);
				if (musicIndex == -1) {
					// has no cache == create new BMSNode object
					std::vector<BMSInfoData*> vec(musicCount);
					for (uint8_t i = 0; i < musicCount; ++i) {
						BMSInfoData* temp = new BMSInfoData();
						temp->mSoundExtension = extension;
						mDecryptor.BuildInfoData(temp, patternPathList[i].c_str());
						vec[i] = temp;
					}
					// sort pattern list and add in dictionary
					std::sort(vec.begin(), vec.end(), mPatternSortFunc);
					AddMusic(subPath, std::move(vec));
					bIncMusicNum = true;
					continue;
				}

				// both parent folder and music folder is exist -> check pattern and add if it is new
				std::vector<BMSInfoData*>& vec = musicList[musicIndex].mListData;
				bool* patternChecker = new bool[vec.size()]{};
				bool bIncPatternNum = false;					// variable to check if more than one pattern is added
				for (uint8_t i = 0; i < musicCount; ++i) {
					if (!checkPatternExist(vec, patternPathList[i], patternChecker)) { // new pattren is found
						BMSInfoData* temp = new BMSInfoData();
						mDecryptor.BuildInfoData(temp, patternPathList[i].c_str());
						vec.emplace_back(temp);
						bIncPatternNum = true;
					}
					vec[i]->mSoundExtension = extension;
				}
				// sort pattern list if more than one pattern has been added
				if (bIncPatternNum) {
					std::sort(vec.begin(), vec.end(), mPatternSortFunc);
					mChangeSave = true;
				}
				delete[] patternChecker;
			}

			// sort music list if more than one music has been added
			if (bIncMusicNum) {
				auto& vec = mDicBms[folderPath];
				std::sort(vec.begin(), vec.end(), mMusicSortFunc);
				mChangeSave = true;
			}
			delete[] folderChecker;
		}

		/// <summary> returns the appropriate music sort lambda function for the <paramref name="opt"/> parameter </summary>
		std::function<bool(const BMSNode&, const BMSNode&)> GetMusicSortFunc(SortOption opt) {
			if (opt == SortOption::PATH_ASC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mFolderName < rhs.mFolderName;
				};
			} else if (opt == SortOption::LEVEL_ASC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					if (lhs.mListData[0]->mLevel != rhs.mListData[0]->mLevel) {
						return lhs.mListData[0]->mLevel < rhs.mListData[0]->mLevel;
					}
					return lhs.mFolderName < rhs.mFolderName;
				};
			} else if (opt == SortOption::TITLE_ASC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mTitle < rhs.mListData[0]->mTitle;
				};
			} else if (opt == SortOption::GENRE_ASC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mGenre < rhs.mListData[0]->mGenre;
				};
			} else if (opt == SortOption::ARTIST_ASC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mArtist < rhs.mListData[0]->mArtist;
				};
			} else if (opt == SortOption::BPM_ASC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mBpm < rhs.mListData[0]->mBpm;
				};
			} else if (opt == SortOption::PATH_DEC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mFolderName > rhs.mFolderName;
				};
			} else if (opt == SortOption::LEVEL_DEC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					if (lhs.mListData[0]->mLevel != rhs.mListData[0]->mLevel) {
						return lhs.mListData[0]->mLevel > rhs.mListData[0]->mLevel;
					}
					return lhs.mFolderName > rhs.mFolderName;
				};
			} else if (opt == SortOption::TITLE_DEC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mTitle > rhs.mListData[0]->mTitle;
				};
			} else if (opt == SortOption::GENRE_DEC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mGenre > rhs.mListData[0]->mGenre;
				};
			} else if (opt == SortOption::ARTIST_DEC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mArtist > rhs.mListData[0]->mArtist;
				};
			} else if (opt == SortOption::BPM_DEC) {
				return [](const BMSNode& lhs, const BMSNode& rhs)->bool {
					return lhs.mListData[0]->mBpm > rhs.mListData[0]->mBpm;
				};
			}
		}
		/// <summary> returns the appropriate pattern sort lambda function for the <paramref name="opt"/> parameter </summary>
		std::function<bool(BMSInfoData* const&, BMSInfoData* const&)> GetPatternSortFunc(SortOption opt) {
			if (opt == SortOption::PATH_ASC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mFilePath < rhs->mFilePath;
				};
			} else if (opt == SortOption::LEVEL_ASC) {
				return [](BMSInfoData* const&lhs, BMSInfoData* const& rhs)->bool {
					if (lhs->mLevel != rhs->mLevel) {
						return lhs->mLevel < rhs->mLevel;
					}
					return lhs->mDifficulty < rhs->mDifficulty;
				};
			} else if (opt == SortOption::TITLE_ASC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mTitle < rhs->mTitle;
				};
			} else if (opt == SortOption::GENRE_ASC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mGenre < rhs->mGenre;
				};
			} else if (opt == SortOption::ARTIST_ASC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mArtist < rhs->mArtist;
				};
			} else if (opt == SortOption::BPM_ASC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mBpm < rhs->mBpm;
				};
			} else if (opt == SortOption::PATH_DEC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mFilePath > rhs->mFilePath;
				};
			} else if (opt == SortOption::LEVEL_DEC) {
				return [](BMSInfoData* const&lhs, BMSInfoData* const& rhs)->bool {
					if (lhs->mLevel != rhs->mLevel) {
						return lhs->mLevel > rhs->mLevel;
					}
					return lhs->mDifficulty > rhs->mDifficulty;
				};
			} else if (opt == SortOption::TITLE_DEC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mTitle > rhs->mTitle;
				};
			} else if (opt == SortOption::GENRE_DEC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mGenre > rhs->mGenre;
				};
			} else if (opt == SortOption::ARTIST_DEC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mArtist > rhs->mArtist;
				};
			} else if (opt == SortOption::BPM_DEC) {
				return [](BMSInfoData* const& lhs, BMSInfoData* const& rhs)->bool {
					return lhs->mBpm > rhs->mBpm;
				};
			}
		}
	};
}