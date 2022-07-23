#ifndef IEVENTPROCESSOR_H
#define IEVENTPROCESSOR_H

#include "OtakuMiraiStructures.h"

#include "MiraiCP.hpp"

class ICommandProcessor
{
public:
	virtual ~ICommandProcessor() {};

	virtual EMessageCommandType ProcessorType() { return EMessageCommandType::Unsupported; };
	
	virtual void ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params) {};
};

#endif