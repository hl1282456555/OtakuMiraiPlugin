#include "GenericStringUtils.h"

std::vector<std::string> UGenericStringUtils::SplitIntoArray(const std::string& Source, const std::string& Delimite, bool bCullEmpty /*= true*/)
{
	std::vector<std::string> Result;

	std::string CachedSource = Source;
	std::string::const_iterator SourceIt = CachedSource.cbegin();

	while (SourceIt != CachedSource.cend())
	{
		if (*SourceIt != Delimite.at(0))
		{
			if (std::distance(SourceIt, CachedSource.cend()) < Delimite.length())
			{
				Result.push_back(CachedSource);
				break;
			}
			else
			{
				SourceIt++;
				continue;
			}

		}

		std::string CapturedSubStr = CachedSource.substr(std::distance(CachedSource.cbegin(), SourceIt), Delimite.length());
		if (CapturedSubStr != Delimite)
		{
			SourceIt++;
			continue;
		}

		std::string SplitedStr = CachedSource.substr(0, std::distance(CachedSource.cbegin(), SourceIt));
		if (CachedSource.cbegin() == SourceIt)
		{
			SourceIt = CachedSource.erase(CachedSource.cbegin(), CachedSource.cbegin() + Delimite.length());
		}
		else
		{
			SourceIt = CachedSource.erase(CachedSource.cbegin(), CachedSource.cbegin() + SplitedStr.length() + Delimite.length());

			bool bIsEmptyStr = true;
			std::string::const_iterator SplitedStrIt = SplitedStr.cbegin();
			while (SplitedStrIt != SplitedStr.cend())
			{
				if (*SplitedStrIt != ' ')
				{
					bIsEmptyStr = false;
				}

				SplitedStrIt++;
			}

			if (bIsEmptyStr && bCullEmpty)
			{
				continue;
			}

			Result.push_back(std::move(SplitedStr));
		}
	}

	if (!CachedSource.empty())
	{
		auto SourceIt = CachedSource.cbegin();
		bool bIsEmptyStr = true;
		while (SourceIt != CachedSource.cend())
		{
			if (*SourceIt != ' ')
			{
				bIsEmptyStr = false;
			}

			SourceIt++;
		}

		if (!bIsEmptyStr)
		{
			Result.push_back(CachedSource);
		}
	}

	return Result;
}
