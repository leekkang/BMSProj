#pragma once

#include "dirent.h"

namespace bms {
	constexpr auto ROOT_PATH = L"StreamingAssets";
	constexpr auto CACHE_FILE_NAME = "test.bin";
	#define FOLDER_SYMBOL_CHECK(sym) (wcscmp(sym, CURRENT_FOLDER_SYM) == 0 || wcscmp(sym, PREV_FOLDER_SYM) == 0)

	/// <summary>
	/// A structure that stores a group of bms files for one song (has variable pattern)
	/// </summary>
	struct BMSNode {
		std::wstring mFolderName;
		std::vector<BMSInfoData*> mListData;

		BMSNode(const std::wstring& name, const std::vector<BMSInfoData*>& list) : mFolderName(name), mListData(list) {};
	};

	/// <summary>
	/// A class that stores a group of bms folders and provides convenience functions
	/// </summary>
	class BMSTree {
	public:
		BMSTree() = default;
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

		std::vector<std::string> GetFolderName() {
			std::vector<std::string> v;
			for (auto& e : mDicFolderName) {
				v.emplace_back(e.first);
			}
			return v;
		}

		std::vector<BMSNode> GetList(const std::wstring& key) {
			auto iter = mDicBms.find(key);
			return iter == mDicBms.end() ? std::vector<BMSNode>() : iter->second;
		}

		void SortMusic() {
			// sort std::vector<BMSNode>
		}

		void SortPattern() {
			// sort std::vector<BMSInfoData*>
		}

		/// <summary> save all <see cref="mDicFolderName"/> elements to binary file </summary>
		void Save() {
			if (!mChangeSave) {
				std::cout << "nothing changed" << std::endl;
				return;
			}
			clock_t s = clock();
			std::ofstream os(CACHE_FILE_NAME, std::ios::binary);
			for (const auto& e : mDicBms) {
				for (const auto& node : e.second) {
					std::string uPath = Utility::WideToUTF8(PathAppend(e.first, node.mFolderName));
					uint8_t size = static_cast<uint8_t>(node.mListData.size());
					WriteToBinary(os, uPath);
					WriteToBinary(os, size);
					for (auto info : node.mListData) {
						os << *info;
					}
				}
			}
			os.close();
			std::cout << "save time(ms) : " << std::to_string(clock() - s) << '\n';
		}

		/// <summary>
		/// check file paths and load <see cref="bms::BMSInfoData"/> list from cache file
		/// If the file does not exist or the path is changed, the object is created again to update the list.
		/// </summary>
		void Load(BMSDecryptor& decryptor) {
			clock_t s = clock();
			mChangeSave = false;
			std::unordered_map<std::wstring, std::vector<BMSInfoData*>> cache;	// temporary data storage dictionary
			cache.reserve(1024);
			// load cache data
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
						if (!IsExistFile(PathAppend(wPath, temp->mFileName))) {
							delete temp;
						} else {
							vec.emplace_back(temp);
						}
					}
					cache.emplace(wPath, std::move(vec));
				}
			}
			is.close();
			std::cout << "cache load time(ms) : " << std::to_string(clock() - s) << '\n';

			s = clock();
			// check all subdirectory and create added bms files or folder.
			DIR *dir = OpenDir(ROOT_PATH);
			if (dir) {
				wchar_t* ent;
				while ((ent = ReadDir(dir)) != nullptr) {
					if (FOLDER_SYMBOL_CHECK(ent)) {
						continue;
					}

					if (IsDirectory(dir->info.attrib)) {
						SetFiles(decryptor, PathAppend(ROOT_PATH, ent), true, cache);
					}
				}
			}
			CloseDir(dir);
			std::cout << "filesystem load time(ms) : " << std::to_string(clock() - s) << '\n';

			s = clock();
			// store cached dictionary in mDicBms dictionary
			// The reason for the troublesome process is that it is implemented as a list for sorting music folders.
			for (auto& e : cache) {
				AddMusic(e.first, std::move(e.second));
			}
			// store folder name in mDicFolderName
			for (auto& e : mDicBms) {
				mDicFolderName.emplace(Utility::WideToUTF8(e.first), e.first);
			}
			std::cout << "dictionary copy time(ms) : " << std::to_string(clock() - s) << '\n';
		}

	private:
		/// <summary> variable to check for changes when loading cache files </summary>
		bool mChangeSave;

		/// <summary> A dictionary whose folder name is the key and the path is the value. Used for ui display. </summary>
		std::unordered_map<std::string, std::wstring> mDicFolderName;

		/// <summary>
		/// A dictionary with a folder path as the key and a music folder list as the value.
		/// It is used for music indexing, ui display, etc.
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
		// reference : https://docs.microsoft.com/ko-kr/cpp/c-runtime-library/filename-search-functions?view=vs-2019
		/// <summary> check this attribute mean subdirectory </summary>
		inline bool IsDirectory(const unsigned int attrib) {
			return attrib & 0x10;	 // _A_SUBDIR : 0x10, normal file(_A_ARCH) : 0x20
		}

		/// <summary> add <see cref="bms::BMSNode"/> object into dictionary </summary>
		void AddMusic(const std::wstring& path, std::vector<BMSInfoData*>&& patterns) {
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

		/// <summary> check whether <param name="name"/> is bms file </summary>
		bool IsBmsFile(const std::wstring& name) {
			// bms file extension : .bms, .bme, .bml
			uint16_t len = static_cast<uint16_t>(name.size());
			if (len < 4 || name[len - 4] != '.' || name[len - 3] != 'b' || name[len - 2] != 'm') {
				return false;
			}
			wchar_t w = name[len - 1];
			return w == 's' || w == 'e' || w == 'l';
		}

		/// <summary> find bms file and store in dictionary. if new pattern is found, create new <see cref="bms::BMSInfoData"/> object </summary>
		void SetFiles(BMSDecryptor& decryptor, const std::wstring& path, bool bRootFolder,
					   std::unordered_map<std::wstring, std::vector<BMSInfoData*>>& cache) {
			// check root bms folder (depth 1)
			clock_t s = clock();
			std::vector<std::wstring> bmsList; bmsList.reserve(32);
			DIR *dir = OpenDir(path);
			if (dir) {
				wchar_t* ent;
				while ((ent = ReadDir(dir)) != nullptr) {
					if (FOLDER_SYMBOL_CHECK(ent)) {
						continue;
					}

					std::wstring p = PathAppend(path, ent);
					if (IsDirectory(dir->info.attrib)) {
						if (bRootFolder) {
							SetFiles(decryptor, p, false, cache);
						}
						continue;
					}

					if (IsBmsFile(ent)) {
						bmsList.emplace_back(p);
					}
				}
			}
			CloseDir(dir);

			std::cout << "pattern search time(ms) : " << std::to_string(clock() - s) << '\n';
			// check if this folder is bms music folder
			uint8_t size = static_cast<uint8_t>(bmsList.size());
			if (size == 0) {
				return;
			}

			// add music to dictionary
			auto iter = cache.find(path);	// std::unordered_map<fs::v1::path, uint8_t>::const_iterator
			if (iter == cache.end()) {
				// has no cache == new music
				std::vector<BMSInfoData*> vec(size);
				for (uint8_t i = 0; i < size; ++i) {
					BMSInfoData* temp = new BMSInfoData(GetFileName(bmsList[i]));
					decryptor.BuildInfoData(temp, bmsList[i].c_str());
					vec[i] = temp;
				}
				cache.emplace(path, std::move(vec));
				mChangeSave = true;
				//AddMusic(path, std::move(vec));

				return;
			}

			// check pattern and add if it is new
			s = clock();
			std::vector<BMSInfoData*>& vec = iter->second;
			uint8_t vecSize = static_cast<uint8_t>(vec.size());
			for (uint8_t i = 0; i < size; ++i) {
				std::wstring filename = GetFileName(bmsList[i]);
				uint8_t j = 0;
				for (; j < vecSize; ++j) {
					if (filename == vec[j]->mFileName) {
						break;
					}
				}

				if (j == vecSize) {
					BMSInfoData* temp = new BMSInfoData(filename);
					decryptor.BuildInfoData(temp, bmsList[i].c_str());
					vec.emplace_back(temp);
					mChangeSave = true;
				}
			}
			std::cout << "pattern matching time(ms) : " << std::to_string(clock() - s) << '\n';
		}
	};
}