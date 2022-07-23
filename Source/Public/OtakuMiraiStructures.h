#ifndef OTAKU_MIRAI_STRUCTURES_H
#define OTAKU_MIRAI_STRUCTURES_H

#include <map>
#include <string>

enum class EMessageCommandType
{
	Echo = 0,
	Unsupported
};

static std::map<std::string, EMessageCommandType> MessageCommandTypeNameMap
{ 
	{"echo", EMessageCommandType::Echo}, {"unsupported", EMessageCommandType::Unsupported}
};

#endif