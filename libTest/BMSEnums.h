#pragma once

namespace bms {
	/// <summary> 
	/// specify channel type
	/// reference : http://hitkey.nekokan.dyndns.info/cmds.htm#CHANNEL
	/// </summary>
	enum class Channel : uint16_t {
		BGM = 1,
		MEASURE_LENGTH = 2,
		CHANGE_BPM = 3,
		BGA_BASE = 4,
		BGA_POOR = 6,
		BGA_LAYER = 7,
		CHANGE_BPM_BY_KEY = 8,		// change bpm based on the value set in #BPMxx
		STOP_BY_KEY = 9,
		KEY_1P_1 = 37,				// player1 key number1. Notation in file = 11
		KEY_1P_2 = 38,				// player1 key number2. Notation in file = 12
		KEY_1P_3 = 39,				// player1 key number3. Notation in file = 13
		KEY_1P_4 = 40,				// player1 key number4. Notation in file = 14
		KEY_1P_5 = 41,				// player1 key number5. Notation in file = 15
		KEY_1P_6 = 44,				// player1 key number6. Notation in file = 18
		KEY_1P_7 = 45,				// player1 key number7. Notation in file = 19
		KEY_1P_SCRATCH = 42,		// player1 scratch. Notation in file = 16
		KEY_2P_1 = 73,				// player2 key number1. Notation in file = 21
		KEY_2P_2 = 74,				// player2 key number2. Notation in file = 22
		KEY_2P_3 = 75,				// player2 key number3. Notation in file = 23
		KEY_2P_4 = 76,				// player2 key number4. Notation in file = 24
		KEY_2P_5 = 77,				// player2 key number5. Notation in file = 25
		KEY_2P_6 = 80,				// player2 key number6. Notation in file = 28
		KEY_2P_7 = 81,				// player2 key number7. Notation in file = 29
		KEY_2P_SCRATCH = 78,		// player2 scratch. Notation in file = 26
		KEY_INVISIBLE_START = 109,	// invisible note start point. Used to change the sound of user input keys
									// Notation in file = 31 ~ 39, 41 ~ 49
		KEY_LONG_START = 181,		// longnote start point. Notation in file = 51 ~ 59, 61 ~ 69
		NOT_USED = 252,				// If greater than this value, do not implement. Notation in file = 70
		LANDMINE_START = 469,		// note channel that decrease gauge if user touch. start point. Notation in file = 36 * 13 + 1
		LANDMINE_END = 513,			// note channel that decrease gauge if user touch. end point. Notation in file = 36 * 14 + 9
	};

	/// <summary> specify key type of one bms file </summary>
	enum class KeyType : uint8_t {
		SINGLE_5,
		SINGLE_7,
		DOUBLE_5,  // 5key, 1p + 2p for 1p, 1 guages
		DOUBLE_7,  // 7key, 1p + 2p for 1p, 1 guages
		COUPLE_5,  // 5key, 1p + 2p, 2 guages (rarely used)
		COUPLE_7,  // 7key, 1p + 2p, 2 guages (rarely used)
	};

	/// <summary> specify sorting option of bms music or patterns </summary>
	enum class SortOption : uint8_t {
		PATH_ASC,
		LEVEL_ASC,
		TITLE_ASC,
		GENRE_ASC,
		ARTIST_ASC,
		BPM_ASC,
		PATH_DEC,
		LEVEL_DEC,
		TITLE_DEC,
		GENRE_DEC,
		ARTIST_DEC,
		BPM_DEC,
	};

	/// <summary> specify note type </summary>
	enum class NoteType : uint8_t {
		NORMAL,
		LONG,
		INVISIBLE,	// used to change the sound of user input keys
		LANDMINE	// decrease gauge if user touch correctly
	};
	
	/// <summary> specify long note expression type written in bms file </summary>
	enum class LongnoteType : uint8_t {
		/// <summary> 
		/// Channels 5x and 6x are used, and the value between the first and last notes is filled with zeros.
		/// The end note is a nonzero key value that is meaningless.
		/// ex) #00251:00AA00ZZ -> The long note is specified from the quarter point to the third quarter point of the second bar(measure).
		/// </summary>
		RDM_TYPE_1 = 1,
		/// <summary> 
		/// Channels 1x and 2x are used, and the value between the first and last notes is filled with zeros.
		/// The end note is a nonzero key value, which plays if there is an assigned sound value.
		/// </summary>
		RDM_TYPE_2 = 2,
		/// <summary> 
		/// Channels 5x and 6x are used, filled with the first note value from the first note to the end.
		/// ex) #00251:00AAAAAA -> The long note is specified from the quarter point to the end of the second bar(measure).
		/// caution : obsolete
		/// </summary>
		MGQ_TYPE = 3,
	};

	enum class EncodingType : uint8_t {
		UNKNOWN = 0,
		EUC_KR = 1,			// ANSI, Korean (Expended to CP949)
		SHIFT_JIS = 2,		// ANSI, Japanese
		UTF_8 = 3,			// Unicode Transformation Format
		UTF_8BOM = 4,		// Unicode Transformation Format with Byte Order Mark
		UTF_16BE = 5,		// Universal Character Set Big Endian	 ex) 0x00 0x80
		UTF_16LE = 6,		// Universal Character Set Little Endian	 ex) 0x80 0x00
	};
}