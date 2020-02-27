#include "pch.h"
#include "BMSParser.h"

using namespace BMS;

/// <summary>
/// store parsed line in appropriate variable of <see cref="BMSParser"/> class
/// </summary>
void BMSParser::StoreHeader(std::string line) {
	if (line.rfind("#PLAYER", 0) == 0) {
		mHeader.SetPlayer(std::stoi(line.substr(8)));
	} else if (line.rfind("#GENRE", 0) == 0) {
		mHeader.SetGenre(line.substr(7));
	} else if (line.rfind("#TITLE", 0) == 0) {
		mHeader.SetTitle(line.substr(7));
	} else if (line.rfind("#ARTIST", 0) == 0) {
		mHeader.SetArtist(line.substr(8));
	} else if (line.rfind("#BPM", 0) == 0) {
		if (line[4] == ' ') {
			mHeader.SetBpm(std::stoi(line.substr(5)));
		} else {	// #BPMXX
			std::string index = line.substr(4, 2);
			std::string data = line.substr(7);
			mHeader.AddCmdToList(false, index, data);
		}
	} else if (line.rfind("#PLAYLEVEL", 0) == 0) {
		mHeader.SetLevel(std::stoi(line.substr(11)));
	} else if (line.rfind("#RANK", 0) == 0) {
		mHeader.SetRank(std::stoi(line.substr(6)));
	} else if (line.rfind("#TOTAL", 0) == 0) {
		mHeader.SetTotal(std::stoi(line.substr(7)));
	} else if (line.rfind("#STAGEFILE", 0) == 0) {
		mHeader.SetStageFile(line.substr(11));
	} else if (line.rfind("#BANNER", 0) == 0) {
		mHeader.SetBanner(line.substr(8));
	} else if (line.rfind("#DIFFICULTY", 0) == 0) {
		mHeader.SetDifficulty(std::stoi(line.substr(12)));
	} else if (line.rfind("#WAV", 0) == 0 || line.rfind("#BMP", 0) == 0) {
		std::string key = line.substr(4, 2);
		std::string name = line.substr(7);
		mHeader.AddFileToDict(line[1] == 'W', key, name);
	} else if (line.rfind("#STOP", 0) == 0) {
		std::string index = line.substr(5, 2);
		std::string data = line.substr(8);
		mHeader.AddCmdToList(true, index, data);
	} else if (line.rfind("#LNTYPE", 0) == 0) {

	} else if (line.rfind("#LNOBJ", 0) == 0) {

	}
}

/// <summary>
/// store parsed line in appropriate variable of <see cref="BMSData"/> class
/// </summary>
void BMSParser::StoreData(std::string line) {

}

/// <summary>
/// parse bms file in path and store data to <see cref="BMSParser"/> and <see cref="BMSData"/> class
/// </summary>
bool BMSParser::Parse(std::string path) {
	std::ifstream file(path.data());
	if (!file.is_open()) {
		std::cout << "The file does not exist in this path ; " << path << std::endl;
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

		line = Utility::rtrim(line);
		if (isHeader) {
			StoreHeader(line);
		} else {
			StoreData(line);
		}
	}

	file.close();
	return true;
}