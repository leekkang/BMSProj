#pragma once

#include "BMSData.h"

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

		inline EncodingType GetEncodeType() {
			return mType;
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

				str.append(Utility::WideToUTF8(temp));
				/*int utfLen = WideCharToMultiByte(CP_UTF8, 0, temp, static_cast<int>(length), NULL, 0, NULL, NULL);
				uint16_t originLen = static_cast<uint16_t>(str.size());
				str.resize(originLen + utfLen);
				WideCharToMultiByte(CP_UTF8, 0, temp, static_cast<int>(length), &str[originLen], utfLen, NULL, NULL);*/
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
}