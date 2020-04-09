#pragma once

#include <codecvt>
//#include <atlstr.h>

// reference : https://jacking75.github.io/cpp_StringEncoding/
//			   https://doitnow-man.tistory.com/211
//			   https://codingtidbit.com/2020/02/09/c17-codecvt_utf8-is-deprecated/
namespace Utility {
	// For convenience, frequently defined locales are statically registered. Decide whether or not to eliminate it later
	static std::locale sKorLoc("Korean");
	static std::locale sJpnLoc("Japanese");

	typedef std::codecvt<wchar_t, char, mbstate_t> cvt_facet;
	/// <summary>
	/// convert Unicode to UTF-8 
	/// caution : deprecated in C++ 17 
	/// </summary>
	inline std::string WideToUTF8(const std::wstring& s) {
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(s);
	}
	/// <summary>
	/// convert UTF-8 to Unicode
	/// caution : deprecated in C++ 17 
	/// </summary>
	inline std::wstring UTF8ToWide(const std::string& s) {
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(s);
	}

	// reference : https://www.codeproject.com/Tips/197097/Converting-ANSI-to-Unicode-and-back-2
	/// <summary> convert ANSI to Unicode </summary>
	inline std::wstring AnsiToWide(const std::string& s, const std::locale& loc) {
		uint16_t bufSize = static_cast<uint16_t>(s.length());
		if (bufSize == 0) {
			return std::wstring();
		}

		const cvt_facet& facet = std::use_facet<cvt_facet>(loc);
		mbstate_t state = {0};

		wchar_t* buf = new wchar_t[bufSize];
		const char* ipc = &s[0];
		wchar_t* opc = 0;
		cvt_facet::result result = facet.in(state, ipc, ipc + bufSize, ipc, buf, buf + bufSize, opc);

		if (result != cvt_facet::ok) {
			delete[] buf;
			return std::wstring();
		}

		std::wstring ws(buf, opc - buf);
		delete[] buf;

		return ws;
	}
	/// <summary> convert Unicode to ANSI </summary>
	inline std::string WideToAnsi(const std::wstring& ws, const std::locale& loc) {
		uint16_t bufSize = static_cast<uint16_t>(ws.length() * 2);
		if (bufSize == 0) {
			return std::string();
		}

		const cvt_facet& facet = std::use_facet<cvt_facet>(loc);
		mbstate_t state = {0};

		char* buf = new char[bufSize];
		const wchar_t* ipc = &ws[0];
		char* opc = 0;
		cvt_facet::result result = facet.out(state, ipc, ipc + ws.length(), ipc, buf, buf + bufSize, opc);

		if (result != cvt_facet::ok) {
			delete[] buf;
			return std::string();
		}

		std::string s(buf, opc - buf);
		delete[] buf;

		return s;
	}
	/// <summary> convert Multibyte(ANSI) to UTF-8 </summary>
	inline std::string ToUTF8(const std::string& s, const std::locale& loc) {
		std::wstring ws = AnsiToWide(s, loc);
		return ws.size() == 0 ? std::string() : WideToUTF8(ws);
	}
	/// <summary> convert UTF-8 to Multibyte(ANSI) </summary>
	inline std::string ToAnsi(const std::string& s, const std::locale& loc) {
		return WideToAnsi(UTF8ToWide(s), loc);
	}

	// reference : https://stackoverflow.com/questions/28270310/how-to-easily-detect-utf8-encoding-in-the-string
	/// <summary> check if it is UTF-8 string </summary>
	inline bool IsValidUTF8(const char* str) {
		if (!str) return true;

		const unsigned char* bytes = (const unsigned char *)str;
		uint32_t cp;
		uint8_t num;

		while (*bytes != 0x00) {
			if ((*bytes & 0x80) == 0x00) {
				// U+0000 to U+007F 
				cp = (*bytes & 0x7F);
				num = 1;
			} else if ((*bytes & 0xE0) == 0xC0) {
				// U+0080 to U+07FF 
				cp = (*bytes & 0x1F);
				num = 2;
			} else if ((*bytes & 0xF0) == 0xE0) {
				// U+0800 to U+FFFF 
				cp = (*bytes & 0x0F);
				num = 3;
			} else if ((*bytes & 0xF8) == 0xF0) {
				// U+10000 to U+10FFFF 
				cp = (*bytes & 0x07);
				num = 4;
			} else
				return false;

			bytes += 1;
			for (uint8_t i = 1; i < num; ++i) {
				if ((*bytes & 0xC0) != 0x80)
					return false;
				cp = (cp << 6) | (*bytes & 0x3F);
				bytes += 1;
			}

			if ((cp > 0x10FFFF) ||
				((cp >= 0xD800) && (cp <= 0xDFFF)) ||
				((cp <= 0x007F) && (num != 1)) ||
				((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
				((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
				((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
				return false;
		}

		return true;
	}

	/*
	/// <summary> convert ANSI to Unicode </summary>
	inline std::wstring AnsiToWide(const std::string& s) {
		if (s.empty()) {
			return std::wstring();
		}
		const char* ansi = s.data();
		int ansiLen = static_cast<int>(s.length());

		int uniLen = MultiByteToWideChar(CP_ACP, 0, ansi, ansiLen, NULL, 0);
		std::wstring result(uniLen, 0);
		MultiByteToWideChar(CP_ACP, 0, ansi, ansiLen, &result[0], uniLen);

		return result;
	}

	/// <summary> convert Unicode to ANSI </summary>
	inline std::string WideToAnsi(const std::wstring& s) {
		if (s.empty()) {
			return std::string();
		}
		const wchar_t* uni = s.data();
		int uniLen = static_cast<int>(s.length());

		int ansiLen = WideCharToMultiByte(CP_ACP, 0, uni, uniLen, NULL, 0, NULL, NULL);
		std::string result(ansiLen, 0);
		WideCharToMultiByte(CP_ACP, 0, uni, uniLen, &result[0], ansiLen, NULL, NULL);

		return result;
	}

	/// <summary> convert Unicode to UTF-8 </summary>
	inline std::string WideToUTF8(const std::wstring& s) {
		if (s.empty()) {
			return std::string();
		}
		const wchar_t* uni = s.data();
		int uniLen = static_cast<int>(s.length());

		int utfLen = WideCharToMultiByte(CP_UTF8, 0, uni, uniLen, NULL, 0, NULL, NULL);
		std::string result(utfLen, 0);
		WideCharToMultiByte(CP_UTF8, 0, uni, uniLen, &result[0], utfLen, NULL, NULL);

		return result;
	}

	/// <summary> convert UTF-8 to Unicode </summary>
	inline std::wstring UTF8ToWide(const std::string& s) {
		if (s.empty()) {
			return std::wstring();
		}
		const char* utf = s.data();
		int utfLen = static_cast<int>(s.length());

		int uniLen = MultiByteToWideChar(CP_UTF8, 0, utf, utfLen, NULL, 0);
		std::wstring result(uniLen, 0);
		MultiByteToWideChar(CP_UTF8, 0, utf, utfLen, &result[0], uniLen);

		return result;
	}
	
	/// <summary> convert Multibyte(ANSI) to UTF-8 </summary>
	inline std::string ToUTF8(const std::string& s) {
		int uniLen = 0, utfLen = 0, ansiLen = static_cast<int>(s.length());
		const char* ansi = s.data();
		wchar_t* uni = NULL;
		char* utf = NULL;

		if ((uniLen = MultiByteToWideChar(CP_ACP, 0, ansi, ansiLen, NULL, 0)) <= 0)
			return 0;

		uni = new wchar_t[uniLen + 1];
		memset(uni, 0x00, sizeof(wchar_t)*(uniLen + 1));

		// str ---> unicode
		uniLen = MultiByteToWideChar(CP_ACP, 0, ansi, ansiLen, uni, uniLen);

		if ((utfLen = WideCharToMultiByte(CP_UTF8, 0, uni, uniLen, NULL, 0, NULL, NULL)) <= 0) {
			delete[] uni;
			return 0;
		}

		utf = new char[utfLen + 1];
		memset(utf, 0x00, sizeof(char)*(utfLen + 1));

		// unicode ---> utf8
		utfLen = WideCharToMultiByte(CP_UTF8, 0, uni, uniLen, utf, utfLen, NULL, NULL);
		utf[utfLen] = 0;

		std::string result(utf);
		delete[] uni;
		delete[] utf;

		return result;

	}
	/// <summary> convert UTF-8 to Multibyte(ANSI) </summary>
	inline std::string ToAnsi(const std::string& s) {
		int uniLen = 0, ansiLen = 0, utfLen = static_cast<int>(s.length());
		const char* utf = s.data();
		wchar_t* uni = NULL;
		char* ansi = NULL;

		if ((uniLen = MultiByteToWideChar(CP_UTF8, 0, utf, utfLen, NULL, 0)) <= 0)
			return 0;

		uni = new wchar_t[uniLen + 1];
		memset(uni, 0x00, sizeof(wchar_t)*(uniLen + 1));

		// utf8 --> unicode
		uniLen = MultiByteToWideChar(CP_UTF8, 0, utf, utfLen, uni, uniLen);

		if ((ansiLen = WideCharToMultiByte(CP_ACP, 0, uni, uniLen, NULL, 0, NULL, NULL)) <= 0) {
			delete[] uni;
			return 0;
		}

		ansi = new char[ansiLen + 1];
		memset(ansi, 0x00, sizeof(char)*(ansiLen + 1));

		// unicode --> str
		ansiLen = WideCharToMultiByte(CP_ACP, 0, uni, uniLen, ansi, ansiLen, NULL, NULL);
		ansi[ansiLen] = 0;

		std::string result(ansi);
		delete[] uni;
		delete[] ansi;

		return result;
	}
	*/
	/*
	// reference : https://stackoverflow.com/questions/17562736/how-to-convert-from-utf-8-to-str-using-standard-c
	inline std::string StringToUTF8(const std::string& str, const std::locale& loc = std::locale{}) {
		using wcvt = std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t>;
		std::u32string wstr(str.size(), U'\0');
		std::use_facet<std::ctype<char32_t>>(loc).widen(str.data(), str.data() + str.size(), &wstr[0]);
		return wcvt{}.to_bytes(
			reinterpret_cast<const int32_t*>(wstr.data()),
			reinterpret_cast<const int32_t*>(wstr.data() + wstr.size())
		);
	}
	inline std::string StringFromUTF8(const std::string& str, const std::locale& loc = std::locale{}) {
		using wcvt = std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t>;
		auto wstr = wcvt{}.from_bytes(str);
		std::string result(wstr.size(), '0');
		std::use_facet<std::ctype<char32_t>>(loc).narrow(
			reinterpret_cast<const char32_t*>(wstr.data()),
			reinterpret_cast<const char32_t*>(wstr.data() + wstr.size()),
			'?', &result[0]);
		return result;
	}
	*/
}