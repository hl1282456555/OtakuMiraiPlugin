#include "CommandProcessor_FFXIV.h"
#include "OtakuEventManager.h"
#include "GenericStringUtils.h"

#if WITH_HTTP_REQUEST
#include <cpr/cpr.h>
#endif

#define REGISTER_FFXIV_COMMAND_PROCESSOR(CommandType) \
CommandProcessor[EFFXIVCommandType::##CommandType] = std::bind(&FCommandProcessor_FFXIV::ProcessCommand_##CommandType, this, std::placeholders::_1, std::placeholders::_2);

REGISTER_COMMAND_PROCESSOR(FFXIV);

FCommandProcessor_FFXIV::FCommandProcessor_FFXIV()
{
	BindAllCommandProcessors();
}

FCommandProcessor_FFXIV::~FCommandProcessor_FFXIV()
{

}

EMessageCommandType FCommandProcessor_FFXIV::ProcessorType()
{
	return EMessageCommandType::FFIXV;
}

void FCommandProcessor_FFXIV::ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params)
{
	std::vector<std::string> Arguments = UGenericStringUtils::SplitIntoArray(Params, " ");

	if (Arguments.empty())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，参数解析失败，请重新尝试！"), Event->message.source.value());
		return;
	}

	if (FFXIVCommandTypeNameMap.find(Arguments.front()) == FFXIVCommandTypeNameMap.cend())
	{
		Event->botlogger.info("FFXIV command ", Arguments.front(), " is not support yet.");
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，目前暂时还不支持这个功能，请联系开发者添加支持！"), Event->message.source.value());
		return;
	}

	EFFXIVCommandType CommandType = FFXIVCommandTypeNameMap[Arguments.front()];
	if (CommandProcessor.find(CommandType) == CommandProcessor.cend())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，目前暂时还不支持这个功能，请联系开发者添加支持！"), Event->message.source.value());
		return;
	}

	CommandProcessor[CommandType](Event, Arguments);
}

void FCommandProcessor_FFXIV::BindAllCommandProcessors()
{
	REGISTER_FFXIV_COMMAND_PROCESSOR(ListWorlds);
}

void FCommandProcessor_FFXIV::ProcessCommand_ListWorlds(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments)
{
#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		cpr::Response Response = cpr::Get(cpr::Url{ "https://universalis.app/api/v2/worlds" });
		if (Response.status_code != 200 || Response.text.empty())
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
			return;
		}

		nlohmann::json WorldsJson = nlohmann::json::parse(Response.text);
		if (!WorldsJson.is_array())
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
			return;
		}

		int LineCount = 0, WorldId = 0;
		std::string ResponseMessage;

		for (auto JsonItem : WorldsJson)
		{
			WorldId = JsonItem["id"];
			ResponseMessage += std::to_string(WorldId);
			ResponseMessage += " ---------- ";
			ResponseMessage += JsonItem["name"];
			ResponseMessage += "\r\n";

			LineCount = LineCount == 4 ? 0 : LineCount + 1;
		}

		Event->group.quoteAndSendMessage(MiraiCP::PlainText(ResponseMessage), Event->message.source.value());
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
	}
	
#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
#endif
}

void FCommandProcessor_FFXIV::ProcessCommand_MarketItem(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments)
{
#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
	}

#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
#endif
}
