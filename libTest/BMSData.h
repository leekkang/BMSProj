#pragma once

#include "BMSHeader.h"
#include "BMSBody.h"
#include "Utility.h"

#include <string>

namespace bms {
	/// <summary>
	/// a data structure include all information of <see cref="mPath"/> file
	/// this class has <see cref="BMS::BMSHeader"/> class and <see cref="BMS::BMSBody"/> class as member
	/// </summary>
	class BMSData {
	public:
		BMSData() { std::cout << "BMSData constructor" << std::endl; };
		~BMSData() = default;
		DISALLOW_COPY_AND_ASSIGN(BMSData)
		//BMSData(const BMSData&) = delete;
		//BMSData& operator=(const BMSData&) = delete;
		BMSData(BMSData&& others) noexcept { std::cout << "move:" << std::endl; *this = std::move(others); }
		BMSData& operator=(BMSData&&) noexcept = default;

		bool Make(std::string path);
		void Play();
		void CalculateBeat();

		const std::string& GetFilePath() { return mPath; }
		BMSHeader& GetHeader() { return mHeader; }
		BMSBody& GetBody() { return mBody; }

		void SetFilePath(std::string& path) { mPath = path; }

	private:
		std::string mPath;
		BMSHeader mHeader;
		BMSBody mBody;

	};
}