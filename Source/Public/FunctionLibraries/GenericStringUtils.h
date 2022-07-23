#ifndef GENERIC_STRING_UTILS_H
#define GENERIC_STRING_UTILS_H

#include <string>
#include <vector>

#include "httplib.h"

class UGenericStringUtils
{
private:
	UGenericStringUtils();
public:
	static std::vector<std::string> SplitIntoArray(const std::string& Source, const std::string& Delimite, bool bCullEmpty = true);
};

#endif