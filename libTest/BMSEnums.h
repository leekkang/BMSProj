#pragma once

namespace bms {
	/// <summary> specify channel type </summary>
	enum class Channel : short {
		BGM = 1,
		MEASURE_LENGTH = 2,
		CHANGE_BPM = 3,
		BGA_BASE = 4,
		BGA_POOR = 6,
		BGA_LAYER = 7,
		CHANGE_BPM_BY_KEY = 8,	// change bpm based on the value set in #BPMxx
		STOP_BY_KEY = 9,
		KEY_1P_1 = 37,			// player1 key number1. Notation in file = 11
		KEY_1P_2 = 38,			// player1 key number2. Notation in file = 12
		KEY_1P_3 = 39,			// player1 key number3. Notation in file = 13
		KEY_1P_4 = 40,			// player1 key number4. Notation in file = 14
		KEY_1P_5 = 41,			// player1 key number5. Notation in file = 15
		KEY_1P_6 = 44,			// player1 key number6. Notation in file = 18
		KEY_1P_7 = 45,			// player1 key number7. Notation in file = 19
		KEY_1P_SCRATCH = 42,	// player1 scratch. Notation in file = 16
		KEY_2P_1 = 73,			// player2 key number1. Notation in file = 21
		KEY_2P_2 = 74,			// player2 key number2. Notation in file = 22
		KEY_2P_3 = 75,			// player2 key number3. Notation in file = 23
		KEY_2P_4 = 76,			// player2 key number4. Notation in file = 24
		KEY_2P_5 = 77,			// player2 key number5. Notation in file = 25
		KEY_2P_6 = 80,			// player2 key number6. Notation in file = 28
		KEY_2P_7 = 81,			// player2 key number7. Notation in file = 29
		KEY_2P_SCRATCH = 78,	// player2 scratch. Notation in file = 26
	};

	/// <summary> specify longnote type </summary>
	enum class LongnoteType : char {
		/// <summary> 
		/// Channels 5x and 6x are used, and the value between the first and last notes is filled with zeros.
		/// The end note is a nonzero key value that is meaningless.
		/// ex) #00251:00AA00ZZ -> The long note is specified from the quarter point to the third quarter point of the second bar(measure).
		/// </summary>
		RDM_TYPE_1 = 1,
		/// <summary> 
		/// Channels 5x and 6x are used, filled with the first note value from the first note to the end.
		/// ex) #00251:00AAAAAA -> The long note is specified from the quarter point to the end of the second bar(measure).
		/// </summary>
		MGQ_TYPE = 2,
		/// <summary> 
		/// Channels 1x and 2x are used, and the value between the first and last notes is filled with zeros.
		/// The end note is a nonzero key value, which plays if there is an assigned sound value.
		/// </summary>
		RDM_TYPE_2 = 3,
	};
}