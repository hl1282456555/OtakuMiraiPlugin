#include "CommandProcessor_Roll.h"

#include "OtakuEventManager.h"

#include <boost/container/vector.hpp>
#include <boost/container/string.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

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

	std::string cleanParams = boost::algorithm::erase_all_copy(Params, " ");

	boost::container::vector<boost::container::string> result;
	boost::algorithm::split(result, cleanParams, boost::algorithm::is_any_of("-"));

	if (!result.empty())
	{
		if (result.size() != 2)
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText(helpText), Event->message.source.value());
			return;
		}

		for (int Index = 0; Index < result.size(); ++Index)
		{
			if (!boost::algorithm::all(result.at(Index), boost::algorithm::is_digit()))
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
