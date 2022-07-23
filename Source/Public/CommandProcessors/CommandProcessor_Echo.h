#ifndef COMMAND_PROCESSOR_ECHO_H
#define COMMAND_PROCESSOR_ECHO_H

#include "ICommandProcessor.h"

class FCommandProcessor_Echo : public ICommandProcessor
{
public:
	FCommandProcessor_Echo();
	virtual ~FCommandProcessor_Echo() override;

	virtual EMessageCommandType ProcessorType() override;

	virtual void ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params) override;
};

#endif