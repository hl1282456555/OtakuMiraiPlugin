#include "GenericStringUtils.h"

#include <cwchar>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdio>

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

std::string UGenericStringUtils::FormatTextTableStyle(int FillCount, const std::vector<std::vector<std::string>>& Source)
{
	if (Source.empty())
	{
		return "";
	}

	std::vector<std::string> FinalRow;
	FinalRow.resize(Source.size());

	std::vector<int> ColumnLengths;
	ColumnLengths.resize(Source.size());

	int ColumnCount = Source[0].size();
	for (int ColumnIndex = 0; ColumnIndex < ColumnCount; ColumnIndex++)
	{
		int RowMaximumLength = 0;
		for (int RowIndex = 0; RowIndex < Source.size(); RowIndex++)
		{
			FinalRow[RowIndex].append(Source[RowIndex][ColumnIndex]);
			ColumnLengths[RowIndex] = CalculateTextLength(Source[RowIndex][ColumnIndex]);
			RowMaximumLength = RowMaximumLength < ColumnLengths[RowIndex] ? ColumnLengths[RowIndex] : RowMaximumLength;
		}

		for (int RowIndex = 0; RowIndex < FinalRow.size(); RowIndex++)
		{
			int RemainingCount = RowMaximumLength - ColumnLengths[RowIndex];
			FinalRow[RowIndex].append(RemainingCount, ' ');
			FinalRow[RowIndex].append("\t");
		}
	}

	std::string ResultContent;
	for (auto RowIt = FinalRow.cbegin(); RowIt != FinalRow.cend(); ++RowIt)
	{
		ResultContent.append(*RowIt);
		if ((RowIt + 1) != FinalRow.cend())
		{
			ResultContent.append("\r\n");
		}
	}

	return ResultContent;
}

unsigned int UGenericStringUtils::CalculateTextLength(const std::string& Source)
{
	int RawColumnLength = Source.length();
	int RealColumnLength = 0;

	unsigned int Index = 0;
	while (Index < RawColumnLength)
	{
		Index += std::mblen(&Source[Index], RawColumnLength - Index);
		RealColumnLength++;
	}

	return RealColumnLength;
}

std::string UGenericStringUtils::ConvertTimestamp(long long Timestamp)
{
	std::locale::global(std::locale("zh_CN.utf8"));
	time_t CachedTimestamp = Timestamp * 1000;
	char MbStr[128] = { 0 };
	if (std::strftime(MbStr, sizeof(MbStr), "%A %c", std::localtime((const time_t*)&Timestamp)) == 0)
	{
		return "";
	}

	return std::string(MbStr);
}
