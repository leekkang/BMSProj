#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <fstream>
#include <chrono>

#include "Unicode.h"

#define PRINT_TRACE 0
#define PRINT_LOG 1

#if PRINT_TRACE
#define __FILENAME__ (strrchr(__FILE__,'\\')+1)	// change full path to file name
#define TRACE(msg) { std::cout << "[" << __FILENAME__ << ":" << __FUNCTION__ << "():" << __LINE__ << "] : " << msg << std::endl; }
//#define TRACE(msg) { printf("[%s:%s():%d] : %s\n", __FILENAME__, __FUNCTION__, __LINE__, msg); }
//#define TRACE(msg, ...) { printf("[%s:%s():%d] : "msg"\n", __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__); }
#else
#define TRACE(s, ...) ;
#endif

#if PRINT_LOG
#define LOG(msg) { std::cout << "[" << __FUNCTION__ << "():" << __LINE__ << "] : " << msg << std::endl; }
#else
#define LOG(msg) ;
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) TypeName(const TypeName&) = delete; \
										   TypeName& operator=(const TypeName&) = delete;

namespace Utility {
	constexpr auto TRIM_SPACE = " \t\n\r";
	//#define TRIM_SPACE " \t\n\r"

	template<typename Out>
	constexpr void Split(const std::string &s, char delim, Out result) {
		std::stringstream ss(s);
		std::string item;
		
		while (getline(ss, item, delim))
			*(result++) = item;
	}
	inline std::vector<std::string> Split(const std::string &s, const char delim) {
		std::vector<std::string> elems;
		Split(s, delim, back_inserter(elems));

		return elems;
	}
	
	inline std::string GetDirectory(const std::string& path) {
		size_t found = path.find_last_of("/\\");
		return path.substr(0, found);
	}

	inline std::string trim(std::string& s, const std::string& drop = TRIM_SPACE) {
		std::string r = s.erase(s.find_last_not_of(drop) + 1);
		return r.erase(0, r.find_first_not_of(drop));
	}
	inline std::string Rtrim(std::string& s, const std::string& drop = TRIM_SPACE) {
		return s.erase(s.find_last_not_of(drop) + 1);
	}
	inline std::string Ltrim(std::string& s, const std::string& drop = TRIM_SPACE) {
		return s.erase(0, s.find_first_not_of(drop));
	}

	/// <summary> Simple string compare for prefix </summary>
	constexpr bool StartsWith(const char* str, const char* prefix) {
		while (*prefix && *str) {
			if (*str++ != *prefix++)
				return false;
		}
		return !(*prefix);
	}

	// strtoi source code : https://github.com/gcc-mirror/gcc/blob/master/libiberty/strtol.c
	/// <summary> Simple string parse to integer with no exception </summary>
	constexpr int parseInt(const char* p, const int length = 0, const int radix = 10) noexcept {
		while (*p == ' ') {
			++p;
		}

		int sign = 1;
		if (*p == '-') {
			sign = -1; ++p;
		} else if (*p == '+') {
			++p;
		}

		bool hasLimit = length != 0;
		char c = *p;
		const char* pstart = p;
		int acc = 0;
		while (hasLimit ? (p - pstart) < length : *p) {
			if (*p >= '0' && *p <= '9') {
				c -= '0';
			} else if (*p >= 'A' && *p <= 'Z') {
				c -= 'A' - 10;
			} else if (*p >= 'a' && *p <= 'z') {
				c -= 'a' - 10;
			} else {
				break;
			}
			if (c >= radix) {
				break;
			}
			acc = acc * radix + c;
			c = *++p;
		}

		return sign * acc;
	}
	// strtof source code : https://github.com/ochafik/LibCL/blob/master/src/main/resources/LibCL/strtof.c
	/// <summary> Simple string parse to floating point with no exception </summary>
	constexpr double parseFloat(const char* p) noexcept {
		while (*p == ' ') {
			++p;
		}

		int sign = 1;
		if (*p == '-') {
			sign = -1; ++p;
		} else if (*p == '+') {
			++p;
		}

		double acc = 0;
		while (*p >= '0' && *p <= '9') {
			acc = acc * 10 + *p++ - '0';
		}

		if (*p++ == '.') {
			double k = 0.1;
			while (*p >= '0' && *p <= '9') {
				acc += (*p++ - '0') * k;
				k *= 0.1;
			}
		}

		return sign * acc;
	}

	/// <summary> Simple string to integer with no exception </summary>
	inline int Stoi(const char* s, int base = 10) noexcept {	// convert string to int
		char *_Eptr;
		return static_cast<int>(_CSTD strtol(s, &_Eptr, base));
	}
	// strtof source code : https://github.com/ochafik/LibCL/blob/master/src/main/resources/LibCL/strtof.c
	/// <summary> Simple string to floating point with no exception </summary>
	inline float Stof(const char* s) noexcept {	// convert string to int
		char *_Eptr;
		return _CSTD strtof(s, &_Eptr);
	}

	/// <summary> Find the greatest common divisor recursively (Euclid's Method) </summary>
	constexpr int GCD(const int m, const int n) {
		return n == 0 ? m : GCD(n, m % n);
	}

	/// <summary> Find the least common multiple </summary>
	constexpr int LCM(const int m, const int n) {
		return (m * n) / GCD(m, n);
	}

	// reference : https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
	//			   https://en.wikipedia.org/wiki/Xorshift
	static unsigned long seed = std::chrono::steady_clock::now().time_since_epoch().count();
	static unsigned long y = 362436069, z = 521288629;
	/// <summary> Random number generator </summary>
	inline unsigned long xorshf96() {          //period 2^96-1
		unsigned long t;
		seed ^= seed << 16;
		seed ^= seed >> 5;
		seed ^= seed << 1;
		t = seed;
		seed = y;
		y = z;
		z = t ^ seed ^ y;

		return z;
	}

	inline bool CompareDoubleSimple(double x, double y, double absTolerance = (1.0e-8)) {
		return fabs(x - y) <= absTolerance;
	}

	// Absolute Compare + Unit in the last place(ULP)
	// source : https://m.blog.naver.com/PostView.nhn?blogId=devmachine&logNo=220142313114&targetKeyword=&targetRecommendationCode=1
	inline int CompareDoubleAbsoulteAndUlps(double x,
											double y,
											double absTolerance = (1.0e-8),
											int ulpsTolerance = 4) {
		double diff = x - y;
		if (fabs(diff) <= absTolerance)
			return 0;

		__int64 nx = *((__int64*)&x);
		__int64 ny = *((__int64*)&y);

		if ((nx & 0x8000000000000000) != (ny & 0x8000000000000000))
			return (diff > 0) ? 1 : -1;

		__int64 ulpsDiff = nx - ny;
		if ((ulpsDiff >= 0 ? ulpsDiff : -ulpsDiff) <= ulpsTolerance)
			return 0;

		return (diff > 0) ? 1 : -1;
	}

	// reference : http://veblush.blogspot.com/2012/10/map-vs-unorderedmap-for-string-key.html
	/// <summary> 
	/// A structure defined in unordered_map to use the key value as the hash value
	/// Use it only when the lower b bits of the hash value are evenly distributed
	/// </summary>
	struct Bypass {
		size_t operator()(int v) const {
			return static_cast<size_t>(v);
		}
	};

	/// <summary> Data structures for fraction representation </summary>
	struct Fraction {
		int mNumerator;
		int mDenominator;

		constexpr static Fraction Zero() {
			return Fraction();
		}

		constexpr Fraction() : mNumerator(0), mDenominator(1) {}
		constexpr Fraction(int n, int d) : mNumerator(n / GCD(n, d)), mDenominator(d / GCD(n, d)) {}
		constexpr Fraction(const Fraction& rhs) : mNumerator(rhs.mNumerator), mDenominator(rhs.mDenominator) {
			//std::cout << "copy constructor" << std::endl;
		}
		// No different from copy...
		// caution : if this object is constant and rhs is not defined as as constant, the copy constructor is executed.
		constexpr Fraction(const Fraction&& rhs) : mNumerator(rhs.mNumerator), mDenominator(rhs.mDenominator) {
			//std::cout << "move constructor" << std::endl;
		}

		constexpr void Set(int n, int d) {
			mNumerator = n;
			mDenominator = d;
		}

		constexpr void Clear() {
			mNumerator = 0;
			mDenominator = 1;
		}
		constexpr double GetValue() const {
			return mNumerator == 0 ? 0 : (double)mNumerator / mDenominator;
		}

		// ----- operator overloading -----

		// assignment is not allowed when this object is constant.. below overloading line only works in non-constant object
		constexpr Fraction& operator=(const Fraction& rhs) {
			if (this != &rhs) {
				mNumerator = rhs.mNumerator;
				mDenominator = rhs.mDenominator;
			}
			return *this;
		}
		constexpr Fraction& operator=(const Fraction&& rhs) {
			if (this != &rhs) {
				mNumerator = std::move(rhs.mNumerator);
				mDenominator = std::move(rhs.mDenominator);
			}
			return *this;
		}
		// The const before the function body indicates whether the member variable can be changed.
		constexpr bool operator==(const Fraction& rhs) const {
			return mNumerator * rhs.mDenominator == mDenominator * rhs.mNumerator;
		}
		constexpr bool operator!=(const Fraction& rhs) const {
			return !(*this == rhs);
		}
		constexpr bool operator<(const int val) const {
			return GetValue() < val;
		}
		constexpr bool operator>(const int val) const {
			return GetValue() > val;
		}
		constexpr bool operator<=(const int val) const {
			return GetValue() <= val;
		}
		constexpr bool operator>=(const int val) const {
			return GetValue() >= val;
		}
		constexpr bool operator<(const Fraction& rhs) const {
			return GetValue() < rhs.GetValue();
			//return (*this - rhs) < 0;
		}
		constexpr bool operator>(const Fraction& rhs) const {
			return GetValue() > rhs.GetValue();
			//return (*this - rhs) > 0;
		}
		constexpr Fraction& operator+=(const Fraction& rhs) {
			if (rhs.mNumerator != 0) {
				int lcm = LCM(mDenominator, rhs.mDenominator);
				int n = lcm / mDenominator * mNumerator + lcm / rhs.mDenominator * rhs.mNumerator;
				if (n == 0) {
					Clear();
					return *this;
				}
				int gcd = GCD(lcm, n);
				mNumerator = n / gcd;
				mDenominator = lcm / gcd;
			}
			return *this;
		}
		constexpr Fraction operator+(const Fraction& rhs) const {
			return Fraction(*this) += rhs;
		}
		constexpr Fraction& operator-=(const Fraction& rhs) {
			if (rhs.mNumerator != 0) {
				int lcm = LCM(mDenominator, rhs.mDenominator);
				int n = lcm / mDenominator * mNumerator - lcm / rhs.mDenominator * rhs.mNumerator;
				if (n == 0) {
					Clear();
					return *this;
				}
				int gcd = GCD(lcm, n);
				mNumerator = n / gcd;
				mDenominator = lcm / gcd;
			}
			return *this;
		}
		constexpr Fraction operator-(const Fraction& rhs) const {
			return Fraction(*this) -= rhs;
		}
		constexpr Fraction& operator*=(const Fraction& rhs) {
			if (rhs.mNumerator != 0 && mNumerator != 0) {
				int n = mNumerator * rhs.mNumerator;
				int d = mDenominator * rhs.mDenominator;
				int gcd = GCD(d, n);
				mNumerator = n / gcd;
				mDenominator = d / gcd;
			} else {
				Clear();
			}
			return *this;
		}
		constexpr Fraction operator*(const Fraction& rhs) const {
			return Fraction(*this) *= rhs;
		}
		constexpr Fraction& operator*=(const int val) {
			if (val != 0 && mNumerator != 0) {
				int n = mNumerator * val;
				int gcd = GCD(mDenominator, n);
				mNumerator = n / gcd;
				mDenominator /= gcd;
			} else {
				Clear();
			}
			return *this;
		}
		constexpr Fraction operator*(const int val) {
			return Fraction(*this) *= val;
		}
	};

	constexpr int Pow(int x, int p) {
		return p > 1 ? x * Pow(x, p - 1) : p == 1 ? x : 1;
	}

	// template metaprogramming
	/*template<int X, int P>
	struct Pow {
		enum { result = X * Pow<X, P - 1>::result };
	};
	template<int X>
	struct Pow<X, 0> {
		enum { result = 1 };
	};
	template<int X>
	struct Pow<X, 1> {
		enum { result = X };
	};*/
}