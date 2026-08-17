#pragma once
#include <string>

namespace YimMenu::Data
{
	std::string GetAppDataPath();
	bool DownloadModelsJson();
	bool ExtractModelsJson(); // Keep for backward compatibility
}