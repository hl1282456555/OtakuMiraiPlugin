#include "CommandProcessor_Roll.h"

#include "OtakuEventManager.h"
#include "FunctionLibraries/GenericStringUtils.h"

#include <boost/lexical_cast.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <cctype>

REGISTER_COMMAND_PROCESSOR(Roll);

FCommandProcessor_Roll::FCommandProcessor_Roll()
{

}

FCommandProcessor_Roll::~FCommandProcessor_Roll()
{

}

EMessageCommandType FCommandProcessor_Roll::ProcessorType()
{
	return EMessageCommandType::Roll;
}

void FCommandProcessor_Roll::ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params)
{
	int randomMin = 0, randomMax = 100;

	std::vector<std::string> spaceResult = UGenericStringUtils::SplitIntoArray(Params, " ");

	if (!spaceResult.empty())
	{
		std::vector<std::string> result = UGenericStringUtils::SplitIntoArray(spaceResult[0], "-");

		if (result.size() != 2)
		{
			Event->botlogger.info("Params size error.");
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("使用方法：1. 指定范围内随机[min, max] ：/roll min-max\r\n2. 默认范围随机[0, 100] ：/roll"), Event->message.source.value());
			return;
		}

		for (int Index = 0; Index < result.size(); ++Index)
		{
			Event->botlogger.info("result[", Index, "] : ", result[Index]);
			if (std::count_if(result[Index].cbegin(), result[Index].cend(), [](unsigned char Value) { return !std::isdigit(Value); }) > 0)
			{
				Event->group.quoteAndSendMessage(MiraiCP::PlainText("使用方法：1. 指定范围内随机[min, max] ：/roll min-max\r\n2. 默认范围随机[0, 100] ：/roll"), Event->message.source.value());
				return;
			}
		}

		try {
			randomMin = boost::lexical_cast<int>(result.at(0));
			randomMax = boost::lexical_cast<int>(result.at(1));
		}
		catch (std::exception& Error)
		{
			Event->botlogger.info("Cast error : ", Error.what());
		}
	}

	boost::random::uniform_int_distribution<> dist(randomMin, randomMax);
	boost::random::random_device range;

	int ramdonRoll = dist(range);

	Event->group.quoteAndSendMessage(MiraiCP::PlainText(ramdonRoll), Event->message.source.value());
}
