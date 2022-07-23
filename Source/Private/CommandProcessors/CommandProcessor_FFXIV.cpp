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
	REGISTER_FFXIV_COMMAND_PROCESSOR(MarketItem);
	REGISTER_FFXIV_COMMAND_PROCESSOR(RefreshDCMap);
	REGISTER_FFXIV_COMMAND_PROCESSOR(RefreshItemIds);
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
	if (Arguments.size() < 3)
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("请按照正确的格式查询【/mitem 大区名称 物品名称/物品名称HQ】！"), Event->message.source.value());
		return;
	}

	bool bQueryHQ = false;
	std::string DataCenterName = Arguments[1];
	std::string ItemName = Arguments[2];

#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		std::string QueryUrl("https://universalis.app/api/v2/");

		if (DataCenterMap.find(DataCenterName) == DataCenterMap.cend())
		{
			Event->botlogger.warning("Not found the world id for ", DataCenterName);
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，无法查询到您所输入的服务器！"), Event->message.source.value());
			return;
		}

		size_t HQCheckPos = ItemName.size() - 2;
		if (ItemName.at(HQCheckPos) == 'H' || ItemName.at(HQCheckPos) == 'h' || ItemName.at(HQCheckPos + 1) == 'Q' || ItemName.at(HQCheckPos + 1) == 'q')
		{
			bQueryHQ = true;
			ItemName.erase(ItemName.cbegin() + HQCheckPos, ItemName.cend());
		}

		if (ItemIdMap.find(ItemName) == ItemIdMap.cend())
		{
			Event->botlogger.warning("Not found the item id for ", ItemName);
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，无法查询到您所输入的物品！"), Event->message.source.value());
			return;
		}

		QueryUrl += DataCenterName + "/" + std::to_string(ItemIdMap[ItemName]);

		cpr::Response Response = cpr::Get(cpr::Url{ QueryUrl }, 
			cpr::Parameters{ {"listings", "0"}, {"entries", "10"}, {"hq", bQueryHQ ? "true" : "false"} }
		);

		if (Response.status_code != 200 || Response.text.empty())
		{
			Event->botlogger.warning("Query market board data failed, url : '", Response.url, "', status : ", Response.status_code);
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
			return;
		}

		auto ResponseJson = nlohmann::json::parse(Response.text);
		if (ResponseJson.find("recentHistory") == ResponseJson.cend() || !ResponseJson["recentHistory"].is_array())
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，暂时没有查询到您需要的数据！"), Event->message.source.value());
			return;
		}

		std::string ReplyMessage("服务器\tHQ\t买家\t\t单价\t\t数量\r\n");

		auto HistoryListJson = ResponseJson["recentHistory"];
		for (nlohmann::json HistoryItem : HistoryListJson)
		{
			long PricePerUnit = HistoryItem["pricePerUnit"];
			long TotalPrice = HistoryItem["total"];
			int ItemCount = TotalPrice / PricePerUnit;

			std::string WorldName = HistoryItem["worldName"];
			std::string BuyerName = HistoryItem["buyerName"];

			ReplyMessage += WorldName + "\t" + (bQueryHQ ? "O\t" : "X\t") + BuyerName + "\t";
			ReplyMessage += std::to_string(PricePerUnit) + "\t";
			ReplyMessage += "x" + std::to_string(ItemCount) + "\r\n";
		}

		Event->group.sendMessage(MiraiCP::PlainText(ReplyMessage));
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
	}

#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
#endif
}

void FCommandProcessor_FFXIV::ProcessCommand_RefreshDCMap(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments)
{
#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		std::string QueryRegionUrl("https://universalis.app/api/v2/data-centers");

		cpr::Response Response = cpr::Get(cpr::Url{ QueryRegionUrl });

		if (Response.status_code != 200 || Response.text.empty())
		{
			Event->botlogger.warning("Query list of the region id failed, url : '", Response.url, "', status : ", Response.status_code);
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新大区服务器映射表！"), Event->message.source.value());
			return;
		}

		auto RegionListJson = nlohmann::json::parse(Response.text);
		if (!RegionListJson.is_array())
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新大区服务器映射表！"), Event->message.source.value());
			return;
		}

		for (nlohmann::json RegionItem : RegionListJson)
		{
			DataCenterMap[RegionItem["name"]] = RegionItem["region"];
		}

		Event->group.quoteAndSendMessage(MiraiCP::PlainText("大区服务器映射表成功刷新！"), Event->message.source.value());
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
	}

#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
#endif
}

void FCommandProcessor_FFXIV::ProcessCommand_RefreshItemIds(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments)
{
#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		std::string QueryItemUrl("https://cafemaker.wakingsands.com/item");

		int TotalQueryCount = 13;
		int QueryFailedCount = 0;

		for (int Index = 0; Index < TotalQueryCount; Index++)
		{
			cpr::Response Response = cpr::Get(
				cpr::Url{ QueryItemUrl },
				cpr::Parameters{ {"limit", "3000"}, {"page", std::to_string(Index + 1)}}
			);

			if (Response.status_code != 200 || Response.text.empty())
			{
				QueryFailedCount++;
				continue;
			}

			auto ResponseJson = nlohmann::json::parse(Response.text);
			if (ResponseJson.find("Results") == ResponseJson.cend())
			{
				QueryFailedCount++;
				continue;;
			}

			auto ResultsJson = ResponseJson["Results"];
			if (!ResultsJson.is_array())
			{
				QueryFailedCount++;
				continue;
			}

			for (nlohmann::json ItemInfo : ResultsJson)
			{
				if (!ItemInfo["ID"].is_number() || !ItemInfo["Name"].is_string())
				{
					continue;
				}

				std::string ItemName = ItemInfo["Name"];
				if (ItemName.empty())
				{
					continue;
				}

				int ItemId = ItemInfo["ID"];
				ItemIdMap[ItemName] = ItemId;
			}
		}

		std::string ReplyMessage("物品ID映射表刷新完成！\r\n");
		ReplyMessage += " - 一共调用了" + std::to_string(TotalQueryCount) + "次接口，失败了" + std::to_string(QueryFailedCount) + "次.\r\n";
		ReplyMessage += "一共刷新了" + std::to_string(ItemIdMap.size()) + "个物品ID。";

		Event->group.quoteAndSendMessage(MiraiCP::PlainText(ReplyMessage), Event->message.source.value());
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
	}

#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
#endif
}
