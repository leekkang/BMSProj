#pragma once

#include "Utility.h"
#include "BMSEnums.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace bms {
	/// <summary>
	/// a data structure represents an object inside a <see cref="BMSBody"/> class
	/// smallest unit in this data. music only + note + option
	/// </summary>
	class Object {
	public:
		Object(int measure, Channel channel, int fracIndex, int fracDenom, int val) :
			mMeasure(measure), mChannel(channel), mFractionIndex(fracIndex), mFractionDenom(fracDenom), mValue(val) {}
		~Object() = default;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

		/// <summary>
		/// check <paramref name="other"/> has equal measure, channel, fraction
		/// </summary>
		bool IsSameBeat(Object& other) {
			return other.mMeasure == mMeasure && other.mChannel == mChannel &&
				other.mFractionIndex == mFractionIndex && other.mFractionDenom == mFractionDenom;
		}

		int GetMeasure() { return mMeasure; }
		Channel GetChannel() { return mChannel; }
		int GetFracIndex() { return mFractionIndex; }
		int GetFracDenom() { return mFractionDenom; }
		int GetValue() { return mValue; }

		//void SetValue(int val) { mValue = val; }

	private:
		int mMeasure;			// the measure number, starting at 0 (corresponds to `#000`)
		Channel mChannel;		// value of Channel enum
		int mFractionIndex;		// numerator of the fractional position inside the measure
		int mFractionDenom;		// denominator of the fractional position inside the measure
		int mValue;				// the raw value of the BMS object
	};

	/// <summary>
	/// 
	/// </summary>
	class BMSBody {
	public:
		BMSBody() {};//{ std::cout << "BMSHeader constructor" << std::endl; };
		~BMSBody() = default;

		BMSBody(const BMSBody&) = delete;
		BMSBody& operator=(const BMSBody&) = delete;
		BMSBody(BMSBody&& others) noexcept { *this = std::move(others); }
		BMSBody& operator=(BMSBody&&) noexcept = default;

		/// <summary>
		/// store parsed line in appropriate variable of <see cref="BMS::BMSBody"/> class
		/// </summary>
		void StoreBody(std::string&& line);

	private:
		///<summary> a list of data objects </summary>
		std::vector<Object> mListObj;
		///<summary> pair of STOP command number and data </summary>
		std::unordered_map <int, float> mDicTimeSignature;
	};
}