#include "pch.h"
#include "BMSData.h"

using namespace bms;

BMSData::BMSData(std::string path) : mPlayer(1), mRank(2), mTotal(200), mLongNoteType(LongnoteType::RDM_TYPE_1) {
	std::cout << "BMSData constructor" << std::endl;

	// Do not use it if class contains pointer variables.
	// I don't know why below link throw an error that says an access violation.
	// reference : https://www.sysnet.pe.kr/2/0/4
	//memset(((char *)this) + 4, 0, sizeof(BMSData) - 4);
	// Do not use it if class contains std::vector..!
	//memset(this, 0, sizeof(BMSData));

	mPath = path;
}