#ifndef COMMAND_PROCESSOR_ROLL_H
#define COMMAND_PROCESSOR_ROLL_H

#include "ICommandProcessor.h"

class FCommandProcessor_Roll : public ICommandProcessor
{
public:
	FCommandProcessor_Roll();
	virtual ~FCommandProcessor_Roll() override;

	virtual EMessageCommandType ProcessorType() override;

	virtual void ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params) override;
};

#endif