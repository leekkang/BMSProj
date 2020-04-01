#pragma once

#include <filesystem>

namespace bms {
	constexpr auto ROOT_PATH = L"StreamingAssets";
	constexpr auto CACHE_FILE_NAME = "test.bin";
	namespace fs = std::experimental::filesystem;

	/// <summary>
	/// A structure that stores a group of bms files for one song (has variable pattern)
	/// </summary>
	struct BMSNode {
		std::wstring mFolderName;
		std::vector<BMSInfoData*> mListData;

		BMSNode(const std::wstring& name, const std::vector<BMSInfoData*>& list) : mFolderName(name), mListData(list) {};
		~BMSNode() {
			for (auto data : mListData) {
				delete data;
			}
		}
	};

	/// <summary>
	/// A class that stores a group of bms folders and provides convenience functions
	/// </summary>
	class BMSTree {
	public:
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
			std::ofstream os(CACHE_FILE_NAME, std::ios::binary);
			for (const auto& e : mDicBms) {
				fs::v1::path prefix = e.first;
				for (const auto& node : e.second) {
					std::string uPath = prefix.append(node.mFolderName).u8string();
					uint8_t size = static_cast<uint8_t>(node.mListData.size());
					WriteToBinary(os, uPath);
					WriteToBinary(os, size);
					for (auto info : node.mListData) {
						os << *info;
					}
				}
			}
			os.close();
		}

		/// <summary>
		/// check file paths and load <see cref="bms::BMSInfoData"/> list from cache file
		/// If the file does not exist or the path is changed, the object is created again to update the list.
		/// </summary>
		bool Load(BMSDecryptor& decryptor) {
			mChangeSave = false;
			clock_t s = clock();
			// load cache data
			std::ifstream is(CACHE_FILE_NAME, std::ios::binary);
			//std::unordered_map<fs::v1::path, uint8_t> dicIndex;		// Temporary index dictionary to easily find music folders
			//dicIndex.reserve(1024);
			std::unordered_map<std::wstring, std::vector<BMSInfoData*>> cache;	// temporary data storage dictionary
			cache.reserve(1024);
			if (is.is_open()) {
				fs::v1::path wPath;
				uint8_t size;
				while (is.peek() != std::ifstream::traits_type::eof()) {
					wPath = Utility::UTF8ToWide(ReadFromBinary<std::string>(is));
					size = ReadFromBinary<uint8_t>(is);
					if (!fs::exists(wPath)) {
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
						if (!fs::exists(wPath.append(temp->mFileName))) {
							delete temp;
						} else {
							//dicIndex.emplace(ws, static_cast<uint8_t>(vec.size()));
							vec.emplace_back(temp);
						}
					}

					//AddMusic(ws, std::move(vec));
					cache.emplace(wPath.native(), std::move(vec));
				}
			}
			is.close();
			std::cout << "cache load time(ms) : " << std::to_string(clock() - s) << '\n';

			s = clock();
			// check all subdirectory and create added bms files or folder.
			for (auto& p : fs::directory_iterator(ROOT_PATH)) {
				if (fs::is_directory(p.path())) {
					SetFiles(decryptor, p.path(), true, cache);
				}
			}
			std::cout << "filesystem load time(ms) : " << std::to_string(clock() - s) << '\n';

			s = clock();
			// store cached dictionary in mDicBms dictionary
			// The reason for the troublesome process is that it is implemented as a list for sorting music folders.
			for (auto& e : cache) {
				AddMusic(e.first, std::move(e.second));
			}
			// store folder name in mDicFolderName
			for (auto& e : mDicBms) {
				fs::v1::path path = e.first;
				mDicFolderName.emplace(path.filename().u8string(), e.first);
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

		/// <summary> add <see cref="bms::BMSNode"/> object into dictionary </summary>
		void AddMusic(const fs::v1::path& path, std::vector<BMSInfoData*>&& patterns) {
			std::wstring key = path.parent_path().native();
			if (mDicBms.count(key) == 0) {
				std::vector<BMSNode> vec;
				vec.emplace_back(path.filename().native(), std::move(patterns));
				mDicBms.emplace(key, std::move(vec));
			} else {
				std::vector<BMSNode>& vec = mDicBms[key];
				vec.emplace_back(path.filename().native(), std::move(patterns));
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
		};

		/// <summary> find bms file and store in dictionary. if new pattern is found, create new <see cref="bms::BMSInfoData"/> object </summary>
		void SetFiles(BMSDecryptor& decryptor, const fs::v1::path& path, bool bRootFolder,
					  std::unordered_map<std::wstring, std::vector<BMSInfoData*>>& cache) {
			// check root bms folder (depth 1)
			std::vector<fs::v1::path> bmsList; bmsList.reserve(32);
			for (auto &p : fs::directory_iterator(path)) {
				if (fs::is_directory(p.path())) {
					if (bRootFolder) {
						// check bms folder (depth 2)
						SetFiles(decryptor, p.path(), false, cache);
					}
					continue;
				}

				if (IsBmsFile(p.path().native())) {
					bmsList.emplace_back(p.path());
				}
			}

			// check if this folder is bms music folder
			uint8_t size = static_cast<uint8_t>(bmsList.size());
			if (size == 0) {
				return;
			}

			// add music to dictionary
			auto iter = cache.find(path.native());	// std::unordered_map<fs::v1::path, uint8_t>::const_iterator
			if (iter == cache.end()) {
				// has no cache == new music
				std::vector<BMSInfoData*> vec(size);
				for (uint8_t i = 0; i < size; ++i) {
					BMSInfoData* temp = new BMSInfoData(bmsList[i].filename());
					decryptor.BuildInfoData(temp, path.c_str());
					vec[i] = temp;
				}
				cache.emplace(path.native(), std::move(vec));
				mChangeSave = true;
				//AddMusic(path, std::move(vec));

				return;
			}

			// check pattern and add if it is new
			std::vector<BMSInfoData*>& vec = iter->second;
			uint8_t vecSize = static_cast<uint8_t>(vec.size());
			for (uint8_t i = 0; i < size; ++i) {
				std::wstring filename = bmsList[i].filename();
				uint8_t j = 0;
				for (; j < vecSize; ++j) {
					if (filename == vec[j]->mFileName) {
						break;
					}
				}

				if (j == vecSize) {
					BMSInfoData* temp = new BMSInfoData(filename);
					decryptor.BuildInfoData(temp, path.c_str());
					vec.emplace_back(temp);
					mChangeSave = true;
				}
			}
		}
	};
}