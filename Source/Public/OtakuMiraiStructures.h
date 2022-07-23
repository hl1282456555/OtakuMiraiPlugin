#ifndef OTAKU_MIRAI_STRUCTURES_H
#define OTAKU_MIRAI_STRUCTURES_H

#include <map>
#include <string>

enum class EMessageCommandType
{
	Echo = 0,
	FFIXV = 1,
	Unsupported
};

static std::map<std::string, EMessageCommandType> MessageCommandTypeNameMap
{ 
	{"echo", EMessageCommandType::Echo}, {"ffxiv", EMessageCommandType::FFIXV},
	{"unsupported", EMessageCommandType::Unsupported}
};

enum class EFFXIVCommandType
{
	ListWorlds = 0,
	Unsupported
};

static std::map<std::string, EFFXIVCommandType> FFXIVCommandTypeNameMap
{
	{"list_worlds", EFFXIVCommandType::ListWorlds},
	{"unsupported", EFFXIVCommandType::Unsupported}
};

#endif