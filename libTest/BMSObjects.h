#pragma once

#include "Utility.h"
#include "BMSEnums.h"
#include "Serializer.h"

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

		// time = beat * (60 / bpm), time unit = microsecond per beat
		inline long long GetTime(double bpm) const {
			return static_cast<long long>(std::round((mNumerator * 60000000ll) / (mDenominator * bpm)));
		}
	};
	/// <summary>
	/// a data structure that store change timing point include sec, bpm, beats at a certain point
	/// </summary>
	struct TimeSegment {
		TimeSegment() : mCurTime(0), mCurBpm(0) {}
		TimeSegment(long long time, int bpm, int beatNum, int beatDenum) :
					mCurTime(time), mCurBpm(bpm), mCurBeat(beatNum, beatDenum) {}

		long long mCurTime;				// the time at which bpm changes (include start point)
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
		Note() : mKey(0), mChannel(Channel::BGM), mTime(0) {}
		Note(int key, Channel ch, long long time, const BeatFraction& beat) : mKey(key), mChannel(ch), mTime(time), mBeat(beat) {}
		Note(const Note&) = default;
		Note& operator=(const Note&) = default;
		Note(Note&&) noexcept = default;
		Note& operator=(Note&&) noexcept = default;

		int mKey;
		//std::string mFilename;
		Channel mChannel;
		BeatFraction mBeat;
		long long mTime;
	};

	/// <summary>
	/// a data structure represents an object visible to the player on tracks on the screen
	/// include base note, end beat(if it is long note), visibility
	/// </summary>
	struct PlayerNote : Note {
		// ----- constructor, operator overloading -----

		PlayerNote() : Note(), mType(NoteType::NORMAL) {}
		PlayerNote(int key, Channel ch, long long time, const BeatFraction& beat, NoteType type) : Note(key, ch, time, beat), mType(type) {}
		PlayerNote(const PlayerNote& other) : 
			Note(other), mType(other.mType), mEndBeat(other.mEndBeat) {}
		PlayerNote& operator=(const PlayerNote& other) = default;
		PlayerNote(PlayerNote&& other) : 
			Note(other), mType(other.mType), mEndBeat(other.mEndBeat) {}

		NoteType mType;			// tells what this note describes
		BeatFraction mEndBeat;	// if object indicate long note, this variable has non-zero value

		inline bool IsLongNote() { return mEndBeat.mNumerator != 0; }
	};


	// -- bms struct serializer overriding --

	inline void WriteToBinaryImpl(std::ostream& os, const bms::LongnoteType& v) {
		WriteToBinary<char>(os, static_cast<char>(v));
	}
	inline void ReadFromBinaryImpl(std::istream& is, bms::LongnoteType& v) {
		v = static_cast<bms::LongnoteType>(ReadFromBinary<char>(is));
	}

	inline void WriteToBinaryImpl(std::ostream& os, const bms::BeatFraction& v) {
		if (!os.write(reinterpret_cast<const char*>(&v), sizeof(bms::BeatFraction))) {
			throw std::ios_base::failure(std::string{"writing type 'bms::BeatFraction' failed"});
		}
	}
	inline void ReadFromBinaryImpl(std::istream& is, bms::BeatFraction& v) {
		if (!is.read(reinterpret_cast<char*>(&v), sizeof(bms::BeatFraction))) {
			throw std::ios_base::failure(std::string{"reading type 'bms::BeatFraction' failed"});
		}
	}

	inline void WriteToBinaryImpl(std::ostream& os, const bms::TimeSegment& v) {
		if (!os.write(reinterpret_cast<const char*>(&v), sizeof(bms::TimeSegment))) {
			throw std::ios_base::failure(std::string{"writing type 'bms::TimeSegment' failed"});
		}
		/*WriteToBinary<long long>(os, v.mCurTime);
		WriteToBinary<double>(os, v.mCurBpm);
		WriteToBinary<bms::BeatFraction>(os, v.mCurBeat);*/
	}
	inline void ReadFromBinaryImpl(std::istream& is, bms::TimeSegment& v) {
		if (!is.read(reinterpret_cast<char*>(&v), sizeof(bms::TimeSegment))) {
			throw std::ios_base::failure(std::string{"reading type 'bms::TimeSegment' failed"});
		}
		/*v.mCurTime = ReadFromBinary<long long>(is);
		v.mCurBpm = ReadFromBinary<double>(is);
		v.mCurBeat = ReadFromBinary<bms::BeatFraction>(is);*/
	}

	inline void WriteToBinaryImpl(std::ostream& os, const bms::Note& v) {
		if (!os.write(reinterpret_cast<const char*>(&v), sizeof(bms::Note))) {
			throw std::ios_base::failure(std::string{"writing type 'bms::Note' failed"});
		}
	}
	inline void ReadFromBinaryImpl(std::istream& is, bms::Note& v) {
		if (!is.read(reinterpret_cast<char*>(&v), sizeof(bms::Note))) {
			throw std::ios_base::failure(std::string{"reading type 'bms::Note' failed"});
		}
	}

	inline void WriteToBinaryImpl(std::ostream& os, const bms::PlayerNote& v) {
		if (!os.write(reinterpret_cast<const char*>(&v), sizeof(bms::PlayerNote))) {
			throw std::ios_base::failure(std::string{"writing type 'bms::PlayerNote' failed"});
		}
	}
	inline void ReadFromBinaryImpl(std::istream& is, bms::PlayerNote& v) {
		if (!is.read(reinterpret_cast<char*>(&v), sizeof(bms::PlayerNote))) {
			throw std::ios_base::failure(std::string{"reading type 'bms::PlayerNote' failed"});
		}
	}
}