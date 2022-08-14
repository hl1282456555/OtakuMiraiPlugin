#include "CommandProcessor_Roll.h"

#include "OtakuEventManager.h"

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <ctime>

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
	boost::random::uniform_int_distribution<> dist(1, 100);
	boost::random::random_device range;

	int ramdonRoll = dist(range);

	Event->group.quoteAndSendMessage(MiraiCP::PlainText(ramdonRoll), Event->message.source.value());
}
