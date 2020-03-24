#pragma once

#include <codecvt>
#include <atlstr.h>

// reference : https://jacking75.github.io/cpp_StringEncoding/
//			   https://doitnow-man.tistory.com/211
//			   https://codingtidbit.com/2020/02/09/c17-codecvt_utf8-is-deprecated/
namespace Utility {
	/// <summary> convert ANSI to Unicode </summary>
	inline std::wstring AnsiToWide(const std::string& s) {
		int uniLen = 0, ansiLen = static_cast<int>(s.length());
		const char* ansi = s.data();
		wchar_t* uni = NULL;

		if ((uniLen = MultiByteToWideChar(CP_ACP, 0, ansi, ansiLen, NULL, 0)) <= 0) {
			return 0;
		}

		uni = new wchar_t[uniLen + 1];
		memset(uni, 0x00, sizeof(wchar_t)*(uniLen + 1));

		uniLen = MultiByteToWideChar(CP_ACP, 0, ansi, ansiLen, uni, uniLen);
		uni[uniLen] = 0;

		std::wstring result(uni);
		delete[] uni;

		return result;
	}

	/// <summary> convert Unicode to ANSI </summary>
	inline std::string WideToAnsi(const std::wstring& s) {
		int ansiLen = 0, uniLen = static_cast<int>(s.length());
		const wchar_t* uni = s.data();
		char* ansi = NULL;

		if ((ansiLen = WideCharToMultiByte(CP_ACP, 0, uni, uniLen, NULL, 0, NULL, NULL)) <= 0) {
			return 0;
		}

		ansi = new char[ansiLen + 1];
		memset(ansi, 0x00, sizeof(char)*(ansiLen + 1));

		ansiLen = WideCharToMultiByte(CP_ACP, 0, uni, uniLen, ansi, ansiLen, NULL, NULL);
		ansi[ansiLen] = 0;

		std::string result(ansi);
		delete[] ansi;

		return result;
	}

	// TODO : assert this func -> deprecated in C++ 17
	/*inline std::string WideToUTF8(const std::wstring& s) {
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(s);
	}
	inline std::wstring UTF8ToWide(const std::string& s) {
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(s);
	}*/

	/// <summary> convert Unicode to UTF-8 </summary>
	inline std::string WideToUTF8(const std::wstring& s) {
		if (s.empty()) {
			return std::string();
		}
		int utfLen = WideCharToMultiByte(CP_UTF8, 0, &s[0], static_cast<int>(s.length()), NULL, 0, NULL, NULL);
		std::string result(utfLen, 0);
		WideCharToMultiByte(CP_UTF8, 0, &s[0], static_cast<int>(s.length()), &result[0], utfLen, NULL, NULL);

		return result;
	}

	/// <summary> convert UTF-8 to Unicode </summary>
	inline std::wstring UTF8ToWide(const std::string& s) {
		if (s.empty()) {
			return std::wstring();
		}
		int uniLen = MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.length()), NULL, 0);
		std::wstring result(uniLen, 0);
		MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.length()), &result[0], uniLen);

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

	//inline std::stringstream ConvertWide(const )

	// reference : https://stackoverflow.com/questions/17562736/how-to-convert-from-utf-8-to-str-using-standard-c
	/*inline std::string StringToUTF8(const std::string& str, const std::locale& loc = std::locale{}) {
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
	}*/
}