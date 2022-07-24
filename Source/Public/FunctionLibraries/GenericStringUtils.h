#ifndef GENERIC_STRING_UTILS_H
#define GENERIC_STRING_UTILS_H

#include <string>
#include <vector>

class UGenericStringUtils
{
private:
	UGenericStringUtils();
public:
	static std::vector<std::string> SplitIntoArray(const std::string& Source, const std::string& Delimite, bool bCullEmpty = true);
	static std::string FormatTextTableStyle (int FillCount, const std::vector<std::vector<std::string>>& Source);
	static unsigned int CalculateTextLength(const std::string& Source);
	static std::string ConvertTimestamp(long long Timestamp);
};

#endif