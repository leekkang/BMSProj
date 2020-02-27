#pragma once

namespace BMS {
	enum Channel {
		BGM = 1,
		MEASURE_LENGTH = 2,
		CHANGE_BPM = 3,
		BGA_BASE = 4,
		BGA_POOR = 6,
		BGA_LAYER = 7,
		KEY_SHORT_1 = 37,	// player1 key number1. Notation in file = 11
		KEY_SHORT_2 = 38,	// player1 key number2. Notation in file = 12
		KEY_SHORT_3 = 39,	// player1 key number3. Notation in file = 13
		KEY_SHORT_4 = 40,	// player1 key number4. Notation in file = 14
		KEY_SHORT_5 = 41,	// player1 key number5. Notation in file = 15
		KEY_SHORT_6 = 44,	// player1 key number6. Notation in file = 18
		KEY_SHORT_7 = 45,	// player1 key number7. Notation in file = 19
		KEY_SHORT_SCRATCH = 42,	// player1 scratch. Notation in file = 16
		KEY_LONG_1 = 181,	// player1 key number1. Notation in file = 51
		KEY_LONG_2 = 182,	// player1 key number2. Notation in file = 52
		KEY_LONG_3 = 183,	// player1 key number3. Notation in file = 53
		KEY_LONG_4 = 184,	// player1 key number4. Notation in file = 54
		KEY_LONG_5 = 185,	// player1 key number5. Notation in file = 55
		KEY_LONG_6 = 188,	// player1 key number6. Notation in file = 58
		KEY_LONG_7 = 189,	// player1 key number7. Notation in file = 59
		KEY_LONG_SCRATCH = 186,	// player1 scratch. Notation in file = 56
	};

	class BMSData {
	public:
	private:
	};

	/// <summary>
	/// parse bms file in path and store data to <see cref="BMSParser"/> and <see cref="BMSData"/> class
	/// </summary>
	class Measure {
	public:
	private:

	};
}