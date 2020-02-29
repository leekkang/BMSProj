#pragma once

#include "BMSData.h"
#include "Utility.h"

#include <fstream>

namespace bms {
	/// <summary>
	/// a class that reads bms file at <c>path</c> and stores it in <see cref="BMS::BMSData"/> class
	/// </summary>
	class Reader {
	public:
		Reader() {}
		~Reader() {}

		/// <summary>
		/// parse bms file in path and store data to <see cref="BMS::Reader"/> and <see cref="BMS::BMSData"/> class
		/// </summary>
		bool Read(std::string&& path);

	private:
		BMSData mData;
	};
}