#include "pch.h"
#include "Reader.h"

using namespace bms;

/// <summary>
/// parse bms file in path and store data to <see cref="BMS::Reader"/> and <see cref="BMS::BMSData"/> class
/// </summary>
bool Reader::Read(std::string&& path) {
	std::ifstream file(path.data());
	if (!file.is_open()) {
		std::cout << "The file does not exist in this path : " << path << std::endl;
		return false;
	}
	
	bool isHeader = true;
	std::string line;
	while (std::getline(file, line)) {
		bool bTrashLine = line[0] != '#';
		if (line.empty() || bTrashLine) {
			// separate header line and data line
			if (bTrashLine && isHeader && line.rfind("*---------------------- MAIN DATA FIELD", 0) == 0) {
				isHeader = false;
			}

			continue;
		}

		line = Utility::Rtrim(line);
		if (isHeader) {
			mData.GetHeader().StoreHeader(std::move(line));
		} else {
			mData.GetBody().StoreBody(std::move(line));
		}
	}

	// sort objects and calculate total beat and measure
	mData.CalculateBeat();
	mData.SetFilePath(path);

	file.close();
	return true;
}