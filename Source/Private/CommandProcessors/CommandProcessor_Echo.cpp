#include "CommandProcessor_Echo.h"

#include "OtakuEventManager.h"

REGISTER_COMMAND_PROCESSOR(Echo);

FCommandProcessor_Echo::FCommandProcessor_Echo()
{

}

FCommandProcessor_Echo::~FCommandProcessor_Echo()
{

}

EMessageCommandType FCommandProcessor_Echo::ProcessorType()
{
	return EMessageCommandType::Echo;
}

void FCommandProcessor_Echo::ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params)
{
	Event->group.sendMessage(Params);
}
