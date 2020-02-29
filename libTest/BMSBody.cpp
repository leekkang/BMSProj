#include "pch.h"
#include "BMSBody.h"

using namespace bms;

/// <summary>
/// store parsed line in appropriate variable of <see cref="BMS::BMSBody"/> class
/// </summary>
void BMSBody::StoreBody(std::string&& line) {
	// example line -> #00116:0010F211
	if (line.size() < 9 || line[6] != ':') {
		TRACE("This data is not in the correct format : " + line);
		return;
	} else if (line.size() == 9 && line[7] == '0' && line[8] == '0') {
		// unnecessary line. -> #xxxxx:00
		return;
	}

	int measure = std::stoi(line.substr(1, 3));
	Channel channel = static_cast<Channel>(std::stoi(line.substr(4, 2), nullptr, 36));
	// create time signature dictionary for calculate beat
	if (channel == Channel::MEASURE_LENGTH) {
		mDicTimeSignature[measure] = std::stof(line.substr(7));
		return;
	}

	clock_t s;
	s = clock();
	int item = line.substr(7).size() / 2;
	for (int i = 0; i < item; ++i) {
		// convert value to base-36, if channel is CHANGE_BPM, convert value to hex
		int val = std::stoi(line.substr(7 + i * 2, 2), nullptr, channel == Channel::CHANGE_BPM ? 16 : 36);
		if (val == 0) {
			continue;
		}
		
		if (channel == Channel::BGM) {
			mListObj.emplace_back(measure, channel, i, item, val);
		} else {
			// overwrite the value (erase and repush)
			Object tmp(measure, channel, i, item, val);
			for (int j = 0; j < mListObj.size(); ++j) {
				if (tmp.IsSameBeat(mListObj[j])) {
					mListObj.erase(mListObj.begin() + j);
					break;
				}
			}
			mListObj.push_back(std::move(tmp));
		}
		TRACE("object create, \n measure : " + std::to_string(measure) + ", fraction : " + 
			  std::to_string(i) + " / " + std::to_string(item) + ", val : " + std::to_string(val))
	}
	TRACE("vector size : " + std::to_string(mListObj.size()) + ", time(ms) : " + std::to_string(clock() - s))
}
