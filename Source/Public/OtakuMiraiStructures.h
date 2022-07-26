#ifndef OTAKU_MIRAI_STRUCTURES_H
#define OTAKU_MIRAI_STRUCTURES_H

#include <map>
#include <string>

enum class EMessageCommandType
{
	Echo = 0,
	FFIXV = 1,
	Roll = 2,
	Unsupported
};

static std::map<std::string, EMessageCommandType> MessageCommandTypeNameMap
{ 
	{"echo", EMessageCommandType::Echo}, {"ffxiv", EMessageCommandType::FFIXV},
	{"roll", EMessageCommandType::Roll},
	{"unsupported", EMessageCommandType::Unsupported}
};

enum class EFFXIVCommandType
{
	ListWorlds = 0,
	MarketItem = 1,
	RefreshDCMap = 2,
	RefreshItemIntro = 3,
	DreamCraystal = 4,
	Unsupported
};

static std::map<std::string, EFFXIVCommandType> FFXIVCommandTypeNameMap
{
	{"list_worlds", EFFXIVCommandType::ListWorlds},
	{"mitem", EFFXIVCommandType::MarketItem},
	{"refresh_dc", EFFXIVCommandType::RefreshDCMap},
	{"refresh_item_intro", EFFXIVCommandType::RefreshItemIntro},
	{"dream_craystal", EFFXIVCommandType::DreamCraystal},
	{"unsupported", EFFXIVCommandType::Unsupported}
};

#endif