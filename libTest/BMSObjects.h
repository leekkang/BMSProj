#pragma once

#include "Utility.h"
#include "BMSEnums.h"

namespace bms {
	/// <summary>
	/// 
	/// </summary>
	struct TimeSegment {
		TimeSegment(double time, int bpm, int beatNum, int beatDenum) :
					mCurTime(time), mCurBpm(bpm), mCurBeat(beatNum, beatDenum) {}

		double mCurTime;				// the time at which bpm changes (include start point)
		double mCurBpm;					// the value of beat per minute. if it is stop point, the value is 0
		Utility::Fraction mCurBeat;		// the beat at which bpm changes (include start point)
	};

	/// <summary>
	/// a data structure represents an object include raw information of bms
	/// smallest unit in this data. music only + note + option
	/// </summary>
	struct Object {
		Object(int measure, Channel channel, int fracIndex, int fracDenom, int val) :
			mMeasure(measure), mChannel(channel), mFraction(fracIndex, fracDenom), mFracIndex(fracIndex), mFracDenom(fracDenom), mValue(val) {}

		int mMeasure;			// the measure number, starting at 0 (corresponds to `#000`)
		Channel mChannel;		// value of Channel enum
		Utility::Fraction mFraction;		// the beat fraction inside the measure
		int mFracIndex;			// numerator of the fractional position inside the measure
		int mFracDenom;			// denominator of the fractional position inside the measure
		int mValue;				// the raw value of the BMS object
	};

	/// <summary>
	/// a data structure represents an object include raw information of bms
	/// smallest unit in this data. music only + note + option
	/// </summary>
	//class Object {
	//public:
	//	// ----- constructor, operator overloading -----

	//	Object(int measure, Channel channel, int fracIndex, int fracDenom, int val) :
	//		mMeasure(measure), mChannel(channel), mFracIndex(fracIndex), mFracDenom(fracDenom), mValue(val) {}
	//	~Object() = default;

	//	Object(const Object&) = delete;
	//	Object& operator=(const Object&) = delete;
	//	Object(Object&&) noexcept = default;
	//	Object& operator=(Object&&) noexcept = default;

	//	// ----- get, set function -----

	//	int GetMeasure() { return mMeasure; }
	//	Channel GetChannel() { return mChannel; }
	//	int GetFracIndex() { return mFracIndex; }
	//	int GetFracDenom() { return mFracDenom; }
	//	int GetValue() { return mValue; }

	////private:
	//	int mMeasure;			// the measure number, starting at 0 (corresponds to `#000`)
	//	Channel mChannel;		// value of Channel enum
	//	int mFracIndex;		// numerator of the fractional position inside the measure
	//	int mFracDenom;		// denominator of the fractional position inside the measure
	//	int mValue;				// the raw value of the BMS object
	//};


	/// <summary>
	/// 
	/// </summary>
	class Note {
	public:
		// ----- constructor, operator overloading -----

		Note() = default;
		~Note() = default;

		Note(const Note&) = delete;
		Note& operator=(const Note&) = delete;
		Note(Note&&) noexcept = default;
		Note& operator=(Note&&) noexcept = default;

		// ----- user access function -----


		// ----- get, set function -----


	private:
	};
}