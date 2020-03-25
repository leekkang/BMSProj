#pragma once

#include "BMSData.h"
#include "BMSObjects.h"

#include <unordered_map>
#include <algorithm>
#include <functional>
#include <stack>

namespace bms {
	/// <summary> Max internal buffer size when read BMS file </summary>
	constexpr auto BUFFER_SIZE = 1024 * 10;
	/// <summary> Max read buffer size when ifstream::read is called </summary>
	constexpr uint16_t READ_BUFFER_SIZE = 1024;

	class BMSifstream {
	public:
		BMSifstream(const wchar_t* path) {
			Open(path);
		};
		~BMSifstream() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSifstream)

		/// <summary> Open ifstream and set the information according to the type of encoding read. </summary>
		bool Open(const wchar_t* path) {
			if (file.is_open()) {
				file.close();
			}

			file.open(path, std::ios_base::binary);
			if (!file.is_open()) {
				TRACE("The file does not exist in this path : " + Utility::WideToAnsi(path));
				return false;
			}
			std::filebuf* pbuf = file.rdbuf();
			pbuf->pubsetbuf(bufInternal, BUFFER_SIZE);

			// caution : Originally, you had to check the length of the file, but give up for speed.
			uint8_t first = pbuf->sbumpc();
			uint8_t second = pbuf->sbumpc();

			// check charset
			// unknown means that the file is one of three types: EUC_KR, SHIFT_JIS, and UTF_8.
			if (first == 254 && second == 255) {
				mType = EncodingType::UTF_16BE;
				getLine = &bms::BMSifstream::getLineUTF16BE;
			} else if (first == 255 && second == 254) {
				mType = EncodingType::UTF_16LE;
				getLine = &bms::BMSifstream::getLineUTF16LE;
			} else {	// include ANSI, UTF-8 no Byte Order Markif 
				if (first == 239 && second == 187 /*&& third == 191*/) {	// skip third character because there is no need to inspect
					mType = EncodingType::UTF_8BOM;
					pbuf->sbumpc();
				} else {
					mType = EncodingType::UNKNOWN;
					pbuf->pubseekpos(0);
				}
				getLine = &bms::BMSifstream::getLineDefault;
			}

			// initialize
			bEof = false;
			readIndex = readCount = READ_BUFFER_SIZE;

			return true;
		}

		inline bool IsOpen() {
			return file.is_open();
		}

		/// <summary> 
		/// Read a line from the file stream and fill it with <paramref name="line"/> parameters.
		/// caution : In case of EncodingType::UTF_16 BE or LE, it is automatically converted to UTF-8.
		/// </summary>
		/// <param name="bSkipEmpty"> if it is true, ignore blank lines and read again </param>
		bool GetLine(std::string& line, bool bSkipEmpty = true) {
			if (bEof) return false;

			if (!line.empty()) line.clear();

			bool bSuccess;
			do {
				bSuccess = (this->*getLine)(line);
			} while (bSkipEmpty && !bEof && line.empty());

			// called when the last sentence of the file is empty.
			if (bSkipEmpty && line.empty()) return false;
			
			return bSuccess;
		}
	private:
		bool bEof;
		EncodingType mType;
		uint16_t readIndex;
		uint16_t readCount;
		bool(bms::BMSifstream::*getLine)(std::string&);

		std::ifstream file;
		char bufInternal[BUFFER_SIZE];
		char bufRead[READ_BUFFER_SIZE];

		/// <summary>
		/// get string from <see cref="bufRead"/>.
		/// When the buffer is finished, <see cref="std::ifstream::read"/> function is called.
		/// </summary>
		bool getLineDefault(std::string& result) {
			uint16_t firstIndex = readIndex;
			do {
				if (readIndex == readCount) {
					if (readIndex != firstIndex) {
						result.append(&bufRead[firstIndex], readIndex - firstIndex);
					}
					file.read(bufRead, READ_BUFFER_SIZE);
					readCount = static_cast<uint16_t>(file.gcount());
					readIndex = 0;
					firstIndex = 0;

					if (bEof = readCount == 0) {
						break;
					}
				}
			} while (bufRead[readIndex++] != '\n');

			if (readIndex <= 1) {
				if (result.size() > 0 && result[result.size() - 1] == '\r') {
					result.pop_back();
				}
			} else {
				uint8_t skip = bufRead[readIndex - 2] == '\r' ? 2 : 1;
				if ((readIndex - skip) != firstIndex) {
					result.append(&bufRead[firstIndex], readIndex - firstIndex - skip);
				}
			}

			return true;
		}

		/// <summary> 
		/// parse buffer char to UTF8 and save in <paramref name="str"/>.
		/// only use for UTF-16 file decrypting.
		/// </summary>
		/// <param name="bIncludedWide"> check if the string needs to be converted. </param>
		void fillStringByUTF8(std::string& str, uint16_t firstIndex, uint16_t length, bool bBigEndian, bool bIncludedWide) {
			if (bIncludedWide) {
				wchar_t* temp = new wchar_t[length];
				if (bBigEndian) {
					for (uint16_t i = 0; i < length; ++i) {
						temp[i] = (bufRead[firstIndex + i * 2] << 4) + bufRead[firstIndex + i * 2 + 1];
					}
				} else {
					for (uint16_t i = 0; i < length; ++i) {
						temp[i] = (bufRead[firstIndex + i * 2 + 1] << 4) + bufRead[firstIndex + i * 2];
					}
				}

				int utfLen = WideCharToMultiByte(CP_UTF8, 0, temp, static_cast<int>(length), NULL, 0, NULL, NULL);
				uint16_t originLen = static_cast<uint16_t>(str.size());
				str.resize(originLen + utfLen);
				WideCharToMultiByte(CP_UTF8, 0, temp, static_cast<int>(length), &str[originLen], utfLen, NULL, NULL);
				delete[] temp;
			} else {
				char* temp = new char[length];
				if (bBigEndian) {
					for (uint16_t i = 0; i < length; ++i) {
						temp[i] = bufRead[firstIndex + i * 2 + 1];
					}
				} else {
					for (uint16_t i = 0; i < length; ++i) {
						temp[i] = bufRead[firstIndex + i * 2];
					}
				}
				str.append(temp, length);
				delete[] temp;
			}
		}
		/// <summary> same as <see cref="getLineDefault"/> function, but use for UTF-16BE. </summary>
		bool getLineUTF16BE(std::string& result) {
			uint16_t firstIndex = readIndex;
			bool bIncludedWide = false;
			char ch;
			do {
				if (readIndex == readCount) {
					if (readIndex != firstIndex) {
						fillStringByUTF8(result, firstIndex, static_cast<uint16_t>((readIndex - firstIndex) * .5), true, bIncludedWide);
						bIncludedWide = false;
					}

					file.read(bufRead, READ_BUFFER_SIZE);
					readCount = static_cast<uint16_t>(file.gcount());
					readIndex = 0;
					firstIndex = 0;

					if (bEof = readCount == 0) {
						break;
					}
				}

				bIncludedWide |= bufRead[readIndex] != '\0';
				ch = bufRead[(++readIndex)++];
			} while (ch != '\n');

			if (readIndex <= 2) {
				if (result.size() > 0 && result[result.size() - 1] == '\r') {
					result.pop_back();
				}
			} else {
				uint8_t skip = bufRead[readIndex - 3] == '\r' ? 4 : 2;
				if ((readIndex - skip) != firstIndex) {
					fillStringByUTF8(result, firstIndex, static_cast<uint16_t>((readIndex - firstIndex - skip) * .5), true, bIncludedWide);
				}
			}

			return true;
		}
		/// <summary> same as <see cref="getLineDefault"/> function, but use for UTF-16LE. </summary>
		bool getLineUTF16LE(std::string& result) {
			uint16_t firstIndex = readIndex;
			bool bIncludedWide = false;
			char ch;
			do {
				if (readIndex == readCount) {
					if (readIndex != firstIndex) {
						fillStringByUTF8(result, firstIndex, static_cast<uint16_t>((readIndex - firstIndex) * .5), false, bIncludedWide);
						bIncludedWide = false;
					}

					file.read(bufRead, READ_BUFFER_SIZE);
					readCount = static_cast<uint16_t>(file.gcount());
					readIndex = 0;
					firstIndex = 0;

					if (bEof = readCount == 0) {
						break;
					}
				}

				ch = bufRead[readIndex];
				bIncludedWide |= bufRead[(++readIndex)++] != '\0';
			} while (ch != '\n');

			if (readIndex <= 2) {
				if (result.size() > 0 && result[result.size() - 1] == '\r') {
					result.pop_back();
				}
			} else {
				uint8_t skip = bufRead[readIndex - 4] == '\r' ? 4 : 2;
				if ((readIndex - skip) != firstIndex) {
					fillStringByUTF8(result, firstIndex, static_cast<uint16_t>((readIndex - firstIndex - skip) * .5), false, bIncludedWide);
				}
			}

			return true;
		}
	};

	/// <summary>
	/// A data structure containing temporary variables and functions required for `bms data` calculations.
	/// This class is declared as a local variable and will not be saved after the whole process.
	/// </summary>
	class BMSDecryptor {
	public:
		// ----- constructor, operator overloading -----

		BMSDecryptor(std::string path) : mBmsData(path) {
			std::cout << "BMSDecryptor constructor for preview" << std::endl;
		};
		BMSDecryptor(std::string path, std::vector<std::string>& data) : mBmsData(path) {
			std::cout << "BMSDecryptor constructor for play" << std::endl; 
			mListRaw = data;
		};
		~BMSDecryptor() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSDecryptor)
		BMSDecryptor(BMSDecryptor&& others) noexcept = default;
		BMSDecryptor& operator=(BMSDecryptor&&) noexcept = default;

		// ----- user access function -----

		/// <summary>
		/// read file and build using data in mBmsData.mPath for fill data in header or body
		/// no file dictionary is created.
		/// </summary>
		/// <returns> return true if all line is correctly saved </returns>
		bool BuildPreview(BMSPreviewData& data) {
			std::ifstream file(mBmsData.mPath.data());
			if (!file.is_open()) {
				TRACE("The file does not exist in this path : " + path);
				return false;
			}

			// declare instant variable for parse
			bool isHeader = true;
			bool isRandom = false;
			std::stack<uint32_t> randomStack;

			std::string line;
			while (std::getline(file, line)) {
				// check incorrect line
				if (line.empty() || line[0] != '#') {
					continue;
				}

				// separate header line and body line
				if (isHeader && line[1] == '0') {	// '0' means that measure 000 is start
					isHeader = false;
				}

				if (isHeader) {
					size_t length = line.size();
					if ((line.rfind("#WAV", 0) == 0 || line.rfind("#BMP", 0) == 0) && length > 7) {
						int key = Utility::Stoi(line.data() + 4, 36);
						line[1] == 'W' ? mBmsData.mDicWav[key] = line.substr(7) :
										 mBmsData.mDicBmp[key] = line.substr(7);
						//TRACE("Store dictionary element : " + std::to_string(key) + ", " + line.substr(7));
					} else if (line.rfind("#BPM", 0) == 0) {
						if (line[4] == ' ' && length > 5) {
							data.mBpm = mBmsData.mMinBpm = mBmsData.mMaxBpm = Utility::Stoi(line.data() + 5);
						} else if (length > 7) {	// #BPMXX
							int key = Utility::Stoi(line.data() + 4, 36);
							mDicBpm[key] = Utility::Stof(line.data() + 7);
						}
					} else if (line.rfind("#STOP", 0) == 0 && length > 8) {
						int key = Utility::Stoi(line.data() + 5, 36);
						mDicStop[key] = Utility::Stoi(line.data() + 8, 36);
					} else if (line.rfind("#PLAYER", 0) == 0 && length > 8) {
						data.mPlayer = Utility::Stoi(line.data() + 8);
					} else if (line.rfind("#PLAYLEVEL", 0) == 0 && length > 11) {
						data.mLevel = std::stoi(line.substr(11));
					} else if (line.rfind("#RANK", 0) == 0 && length > 6) {
						data.mRank = Utility::Stoi(line.data() + 6);
					} else if (line.rfind("#TOTAL", 0) == 0 && length > 7) {
						data.mTotal = Utility::Stoi(line.data() + 7);
					} else if (line.rfind("#GENRE", 0) == 0 && length > 7) {
						data.mGenre = line.substr(7);
					} else if (line.rfind("#TITLE", 0) == 0 && length > 7) {
						data.mTitle = line.substr(7);
					} else if (line.rfind("#ARTIST", 0) == 0 && length > 8) {
						data.mArtist = line.substr(8);
					} else if (line.rfind("#STAGEFILE", 0) == 0 && length > 11) {
						data.mStageFile = line.substr(11);
					} else if (line.rfind("#BANNER", 0) == 0 && length > 8) {
						data.mBannerFile = line.substr(8);
					} else if (line.rfind("#DIFFICULTY", 0) == 0 && length > 12) {
						data.mDifficulty = Utility::Stoi(line.data() + 12);
					} else if (line.rfind("#LNTYPE", 0) == 0 && length > 8) {
						LongnoteType val = static_cast<LongnoteType>(Utility::Stoi(line.data() + 8));
						if (val == LongnoteType::RDM_TYPE_1 || val == LongnoteType::MGQ_TYPE) {
							data.mLongNoteType = val;
						}
					} else if (line.rfind("#LNOBJ", 0) == 0 && length > 7) {
						if (mEndNoteVal != 0) {
							LOG("LNOBJ value is more than one value. : " << line);
						}
						mEndNoteVal = Utility::Stoi(line.data() + 7, 36);
						data.mLongNoteType = LongnoteType::RDM_TYPE_2;
					} else {
						TRACE("This line is not in the correct format : " << line);
					}
				} else {
					ParseBody(Utility::Rtrim(line));
				}
			}

			return true;
		}

		/// <summary>
		/// build using line in <paramref name="lines"/> list for fill data in header or body
		/// </summary>
		/// <returns> return true if all line is correctly saved </returns>
		bool Build();
		/// <summary>
		/// parse <paramref name="line"/> for fill header data and store parsed line in appropriate variable
		/// </summary>
		void ParseHeader(std::string&& line) noexcept;
		/// <summary>
		/// parse <paramref name="line"/> for fill body data and store parsed line in temporary data structure
		/// </summary>
		void ParseBody(std::string&& line) noexcept;
		/// <summary>
		/// make time segment list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::TimeSegment"/> objects
		/// </summary>
		void MakeTimeSegment();
		/// <summary>
		/// make note list in <see cref="bms::BMSData::mListTimeSeg"/> vector contain <see cref="bms::Note"/> objects
		/// </summary>
		void MakeNoteList();

		// ----- get, set function -----

		/// <summary>
		/// return <see cref="BMSData"/> object. It should be called after the build is finished.
		/// </summary>
		inline BMSData&& GetBmsData() { return std::move(mBmsData); }
		//inline BMSData CopyBmsData() { return mBmsData; }

		/// <summary>
		/// Function that returns the number of beats of a particular measure
		/// stored in <see cref="mDicTimeSignature"/> of <see cref="BMSData"/> object.
		/// </summary>
		inline BeatFraction GetBeatInMeasure(int measure) {
			return mDicMeasureLength.count(measure) == 0 ? BeatFraction(4, 1) : mDicMeasureLength[measure] * 4;
		}
		/// <summary>
		/// Function that returns cumulative number of beats at a specific timing
		/// </summary>
		inline BeatFraction GetBeats(int measure, const BeatFraction& frac) {
			if (measure > mEndMeasure) {
				return BeatFraction();
			} else {
				return measure == 0 ? frac * GetBeatInMeasure(measure) :
									  frac * GetBeatInMeasure(measure) + mBmsData.mListCumulativeBeat[measure - 1];
			}
		}
		/// <summary>
		/// Function that returns a time at a specific point using beats.
		/// note : Functions that convert time to beats are not provided because errors can occur during casting.
		/// </summary>
		inline long long GetTimeUsingBeat(const BeatFraction& beat) {
			BeatFraction subtract;
			size_t length = mBmsData.mListTimeSeg.size();
			size_t index = length - 1;
			for (; index > 0; --index) {
				const TimeSegment& t = mBmsData.mListTimeSeg[index];
				subtract = beat - t.mCurBeat;
				// zero bpm means stop signal. skip it.
				if (subtract >= 0) {
					break;
				}
			}

			// previous saved time + current segment time. if index equals to length, add reversed sign
			const TimeSegment& prev = mBmsData.mListTimeSeg[index];
			//TRACE("GetTimeUsingBeat beat area check : [" << (beat > prev.mCurBeat ? 1 : 0) << ", " << 
			//	  (index + 1 < length) ? (beat < mBmsData.mListTimeSeg[index + 1].mCurBeat ? 1 : 0) : 1 << "], prevbpm : " << prev.mCurBpm)
			//bool bTest = (index + 1 < length) ? (mBmsData.mListTimeSeg[index + 1].mCurBeat - beat > 0) : true;
			//std::cout << "GetTimeUsingBeat beat area check : [" << (beat - prev.mCurBeat >= 0) << ", " << bTest << "], prevbpm : " << prev.mCurBpm << std::endl;
			return index == 0 ? beat.GetTime(prev.mCurBpm) :
								prev.mCurTime + subtract.GetTime(prev.mCurBpm);
		}

		/// <summary>
		/// Function that returns a total play time
		/// </summary>
		inline long long GetTotalPlayTime() {
			const TimeSegment& seg = mBmsData.mListTimeSeg[mBmsData.mListTimeSeg.size() - 1];
			BeatFraction subtract = mBmsData.mListCumulativeBeat[mEndMeasure] - seg.mCurBeat;

			return seg.mCurTime + subtract.GetTime(seg.mCurBpm);
		}

	private:
		EncodingType mType;

		/// <summary> The number of total measure of current bms data </summary>
		int mEndMeasure;
		/// <summary> 
		/// if <see cref="mLongNoteType"/> is <see cref="LongnoteType::RDM_TYPE_2"/>, 
		/// this value direct wav file key in <see cref="BMSData::mDicWav"/>. 
		/// </summary>
		int mEndNoteVal;

		BMSData mBmsData;

		///<summary> a list of raw file data not yet parsed </summary>
		std::vector<std::string> mListRaw;
		///<summary> a list of temporary time data objects </summary>
		std::vector<Object> mListRawTiming;

		///<summary> a map of data objects (smallest unit) </summary>
		std::unordered_map<int, std::vector<Object>> mDicObj;

		///<summary> pair of STOP command number and data </summary>
		std::unordered_map<int, int> mDicStop;
		///<summary> pair of BPM command number and data  </summary>
		std::unordered_map<int, float> mDicBpm;
		///<summary> pair of measure number and measure length </summary>
		std::unordered_map<int, BeatFraction> mDicMeasureLength;
	};
}