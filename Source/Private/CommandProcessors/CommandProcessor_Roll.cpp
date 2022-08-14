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

	std::string helpText = "使用方法：1. 指定范围内随机[min, max] ：/roll min-max\r\n2. 默认范围随机[0, 100] ：/roll";

	std::vector<std::string> spaceResult = UGenericStringUtils::SplitIntoArray(Params, " ");

	if (spaceResult.empty())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText(helpText), Event->message.source.value());
		return;
	}

	std::vector<std::string> result = UGenericStringUtils::SplitIntoArray(spaceResult[0], "-");

	Event->botlogger.info("result: ", result.size());

	if (result.size() != 2)
	{
		for (int Index = 0; Index < result.size(); ++Index)
		{
			if (std::count_if(result[Index].cbegin(), result[Index].cend(), [](unsigned char Value) { return !std::isdigit(Value);}) > 0)
			{
				Event->group.quoteAndSendMessage(MiraiCP::PlainText(helpText), Event->message.source.value());
				return;
			}
		}

		randomMin = boost::lexical_cast<int>(result.at(0));
		randomMax = boost::lexical_cast<int>(result.at(1));
	}
	else
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText(helpText), Event->message.source.value());
		return;
	}

	boost::random::uniform_int_distribution<> dist(randomMin, randomMax);
	boost::random::random_device range;

	int ramdonRoll = dist(range);

	Event->group.quoteAndSendMessage(MiraiCP::PlainText(ramdonRoll), Event->message.source.value());
}
