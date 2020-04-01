#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace bms {
	/// <summary> serialize the <see also="T"/> type into a binary stream </summary>
	template<class T> inline void WriteToBinary(std::ostream& os, const T& v);

	/// <summary> deserialize the <see also="T"/> type from a binary stream </summary>
	template<class T> inline void ReadFromBinary(std::istream& is, T& v);

	/// <summary> deserialize the <see also="T"/> type into a binary stream and return value </summary>
	template<class T> inline T ReadFromBinary(std::istream& is);


	// reference : https://bab2min.tistory.com/613
	// Type SFINAE
	// reference : https://github.com/jwvg0425/ModernCppStudy/wiki/SFINAE
	// origin of STL Write Serializer
	template<class T>
	inline typename std::enable_if<std::is_fundamental<T>::value>::type WriteToBinaryImpl(std::ostream& os, const T& v) {
		if (!os.write(reinterpret_cast<const char*>(&v), sizeof(T))) {
			throw std::ios_base::failure(std::string{"writing type '"} +typeid(T).name() + "' failed");
		}
	}

	template<class T>
	inline void WriteToBinaryImpl(std::ostream& os, const typename std::vector<T>& v) {
		// store count of vector element
		WriteToBinary<uint32_t>(os, static_cast<uint32_t>(v.size()));
		for (auto& e : v) {
			WriteToBinary(os, e);
		}
	}
	template<class T>
	inline void WriteToBinaryImpl(std::ostream& os, const typename std::basic_string<T>& v) {
		// store count of char
		WriteToBinary<uint32_t>(os, static_cast<uint32_t>(v.size()));
		for (auto& e : v) {
			WriteToBinary(os, e);
		}
	}
	template<class T1, class T2>
	inline void WriteToBinaryImpl(std::ostream& os, const typename std::pair<T1, T2>& v) {
		WriteToBinary(os, v.first);
		WriteToBinary(os, v.second);
	}
	template<class T1, class T2>
	inline void WriteToBinaryImpl(std::ostream& os, const typename std::unordered_map<T1, T2>& v) {
		// store count of unordered_map element
		WriteToBinary<uint32_t>(os, static_cast<uint32_t>(v.size()));
		for (auto& e : v) {
			WriteToBinary(os, e);
		}
	}

	// origin of STL Read Serializer
	template<class T>
	inline typename std::enable_if<std::is_fundamental<T>::value>::type ReadFromBinaryImpl(std::istream& is, T& v) {
		if (!is.read(reinterpret_cast<char*>(&v), sizeof(T))) {
			throw std::ios_base::failure(std::string{"reading type '"} + typeid(T).name() + "' failed");
		}
	}

	template<class T>
	inline void ReadFromBinaryImpl(std::istream& is, typename std::vector<T>& v) {
		// resize by vector size
		v.resize(ReadFromBinary<uint32_t>(is));
		for (auto& e : v) {
			ReadFromBinary(is, e);
		}
	}
	template<class T>
	inline void ReadFromBinaryImpl(std::istream& is, typename std::basic_string<T>& v) {
		// resize by basic_string size
		v.resize(ReadFromBinary<uint32_t>(is));
		for (auto& e : v) {
			ReadFromBinary(is, e);
		}
	}
	template<class T1, class T2>
	inline void ReadFromBinaryImpl(std::istream& is, typename std::pair<T1, T2>& v) {
		v.first = ReadFromBinary<T1>(is);
		v.second = ReadFromBinary<T2>(is);
	}
	template<class T1, class T2>
	inline void ReadFromBinaryImpl(std::istream& is, typename std::unordered_map<T1, T2>& v) {
		uint32_t len = ReadFromBinary<uint32_t>(is);
		v.clear();
		for (size_t i = 0; i < len; ++i) {
			v.emplace(ReadFromBinary<std::pair<T1, T2>>(is));
		}
	}

	// -- implementation --

	template<class T>
	inline void WriteToBinary(std::ostream& os, const T& v) {
		WriteToBinaryImpl(os, v);
	}

	template<class T>
	inline void ReadFromBinary(std::istream& is, T& v) {
		ReadFromBinaryImpl(is, v);
	}

	template<class T>
	inline T ReadFromBinary(std::istream& is) {
		T v;
		ReadFromBinaryImpl(is, v);
		return v;
	}
}