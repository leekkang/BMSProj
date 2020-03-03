#pragma once

#include "Utility.h"
#include "BMSEnums.h"

namespace bms {
	/// <summary>
	/// Data structure inheriting fraction structure for beat calculation
	/// </summary>
	struct BeatFraction : Utility::Fraction {
		BeatFraction() : Fraction(0, 1) {}
		BeatFraction(int n, int d) : Fraction(n, d) {}
		BeatFraction(const Fraction& rhs) : Fraction(rhs) {}
		// std::move should be used, but is omitted because it performs the same operation as the copy constructor.
		BeatFraction(const Fraction&& rhs) : Fraction(rhs) {}

		// time = beat * (60 / bpm), time unit = second per beat
		inline double GetTime(double bpm) {
			return (mNumerator * 60) / (mDenominator * bpm);
		}
	};
	/// <summary>
	/// a data structure that store change timing point include sec, bpm, beats at a certain point
	/// </summary>
	struct TimeSegment {
		TimeSegment(double time, int bpm, int beatNum, int beatDenum) :
					mCurTime(time), mCurBpm(bpm), mCurBeat(beatNum, beatDenum) {}

		double mCurTime;				// the time at which bpm changes (include start point)
		double mCurBpm;					// the value of beat per minute. if it is stop point, the value is 0
		BeatFraction mCurBeat;			// the beat at which bpm changes (include start point)
	};

	/// <summary>
	/// a data structure represents an object include raw information of bms
	/// smallest unit in this data. music only + note + option
	/// </summary>
	struct Object {
		Object(int val, int measure, Channel channel, int fracIndex, int fracDenom) :
			mValue(val), mMeasure(measure), mChannel(channel), mFraction(fracIndex, fracDenom) {}

		int mValue;						// the raw value of the BMS object
		int mMeasure;					// the measure number, starting at 0 (corresponds to `#000`)
		Channel mChannel;				// value of Channel enum
		BeatFraction mFraction;			// the beat fraction inside the measure
	};


	/// <summary>
	/// a data structure represents an object include sound of bms
	/// include file info, time, beat, Channel
	/// </summary>
	struct Note {
		Note() {}
		Note(int key, Channel ch, double time, const BeatFraction& beat) : mKey(key), mChannel(ch), mTime(time), mBeat(beat) {}
		Note(const Note&) = default;
		Note& operator=(const Note&) = default;
		Note(Note&&) noexcept = default;
		Note& operator=(Note&&) noexcept = default;

		int mKey;
		//std::string mFilename;
		Channel mChannel;
		BeatFraction mBeat;
		double mTime;
	};

	/// <summary>
	/// a data structure represents an object visible to the player on tracks on the screen
	/// include base note, end beat(if it is long note), visibility
	/// </summary>
	struct PlayerNote : Note {
		// ----- constructor, operator overloading -----

		PlayerNote(int key, Channel ch, double time, const BeatFraction& beat, NoteType type) : Note(key, ch, time, beat), mType(type) {}
		PlayerNote(const PlayerNote& other) : 
			Note(other), mType(other.mType), mEndBeat(other.mEndBeat) {}
		PlayerNote(PlayerNote&& other) : 
			Note(other), mType(other.mType), mEndBeat(other.mEndBeat) {}

		NoteType mType;			// tells what this note describes
		BeatFraction mEndBeat;	// if object indicate long note, this variable has non-zero value

		inline bool IsLongNote() { return mEndBeat.mNumerator != 0; }
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
}