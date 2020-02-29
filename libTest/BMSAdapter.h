#pragma once

#include "BMSData.h"

namespace bms {
	/// <summary>
	/// 
	/// </summary>
	class BMSAdapter {
	public:
		// ----- constructor, operator overloading -----

		BMSAdapter() = default;
		~BMSAdapter() {
			//for (BMSData& data : mListData)
		};

		// ----- user access function -----

		/// <summary>
		/// make <see cref="BMS::BMSData"/> object by reading the bms file in <paramref name="path"/>
		/// </summary>
		/// <returns> return true if a <see cref="BMS::BMSData"/> object is correctly build </returns>
		bool Make(std::string path, BMSData& data);

		// ----- get, set function -----
	private:
		///<summary> List that stores the completed bms data instance </summary>
		std::vector<BMSData> mListData;
	};
}