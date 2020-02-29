#pragma once

#include <string>
#include <vector>
#include "BMSData.h"

namespace bms {
	/// <summary>
	/// 
	/// </summary>
	class BMSAdapter {
	public:
		BMSAdapter() = default;
		~BMSAdapter() {
			//for (BMSData& data : mListData)
		};
	private:
		///<summary> List that stores the completed bms data instance </summary>
		std::vector<BMSData&> mListData;
	};
}