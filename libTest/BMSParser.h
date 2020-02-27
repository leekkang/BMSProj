#pragma once

#include "BMSHeader.h"
#include "BMSData.h"
#include "Utility.h"

#include <iostream>
#include <fstream>

namespace BMS {
	class BMSParser {
	public:
		BMSParser() {}
		~BMSParser() {}

		bool Parse(std::string path);

		BMSHeader GetHeader() { return mHeader; }
		BMSData GetData() { return mData; }

		//void SetHeader(const BMSHeader& value) { mHeader = value; }
		//void SetData(const BMSData& value) { mData = value; }

	private:
		BMSHeader mHeader;
		BMSData mData;

		void StoreHeader(std::string line);
		void StoreData(std::string line);
	};
}