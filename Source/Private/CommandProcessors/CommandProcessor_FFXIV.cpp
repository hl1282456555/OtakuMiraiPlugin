#include "CommandProcessor_FFXIV.h"
#include "OtakuEventManager.h"
#include "GenericStringUtils.h"

#include <iomanip>
#include <fstream>
#include <streambuf>
#include <regex>

#include "utf8.h"
#if WITH_HTTP_REQUEST
#include <cpr/cpr.h>
#endif

#include <boost/algorithm/string/replace.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#ifdef __linux__
#define REGISTER_FFXIV_COMMAND_PROCESSOR(CommandType) \
CommandProcessor[EFFXIVCommandType::CommandType] = std::bind(&FCommandProcessor_FFXIV::ProcessCommand_##CommandType, this, std::placeholders::_1, std::placeholders::_2);
#else
#define REGISTER_FFXIV_COMMAND_PROCESSOR(CommandType) \
CommandProcessor[EFFXIVCommandType::##CommandType] = std::bind(&FCommandProcessor_FFXIV::ProcessCommand_##CommandType, this, std::placeholders::_1, std::placeholders::_2);
#endif

REGISTER_COMMAND_PROCESSOR(FFXIV);

FCommandProcessor_FFXIV::FCommandProcessor_FFXIV()
	: MysqlSession(
		mysqlx::SessionOption::HOST, "localhost",
		mysqlx::SessionOption::PORT, 33060,
		mysqlx::SessionOption::USER, "luo",
		mysqlx::SessionOption::PWD, "luoyu@110030",
		mysqlx::SessionOption::DB, "ffxiv_data_schema"
	)
{
	BindAllCommandProcessors();
	InitMySQLDependecies();
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
	REGISTER_FFXIV_COMMAND_PROCESSOR(RefreshItemIntro);
	REGISTER_FFXIV_COMMAND_PROCESSOR(DreamCraystal);
}

void FCommandProcessor_FFXIV::InitMySQLDependecies()
{
	
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

	mysqlx::Schema FFXIVSchema = MysqlSession.getSchema("ffxiv_data_schema");
	if (!FFXIVSchema.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法获取物品信息！"), Event->message.source.value());
		return;
	}

	mysqlx::Table DCTable = FFXIVSchema.getTable("ffxiv_datacenter_table", true);
	if (!DCTable.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法获取物品信息！"), Event->message.source.value());
		return;
	}

	mysqlx::Table ItemIntroTable = FFXIVSchema.getTable("ffxiv_item_intro_table", true);
	if (!ItemIntroTable.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法获取物品信息！"), Event->message.source.value());
		return;
	}

	bool bQueryHQ = false;
	std::string DataCenterName = Arguments[1];
	std::string ItemName = Arguments[2];

#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		std::string QueryUrl("https://universalis.app/api/v2/");
		
		std::string WhereDCSql("dc_name=");
		WhereDCSql += "'" + DataCenterName + "'";

		mysqlx::RowResult DCRowResult = DCTable.select("*").where(WhereDCSql).execute();

		if (DCRowResult.getWarningsCount() > 0 || DCRowResult.count() <= 0)
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

		std::string ItemInfoSql("item_name=");
		ItemInfoSql += "'" + ItemName + "'";
		mysqlx::RowResult ItemRowResult = ItemIntroTable.select("*").where(ItemInfoSql).execute();

		if (ItemRowResult.getWarningsCount() > 0 || ItemRowResult.count() <= 0)
		{
			Event->botlogger.warning("Not found the item id for ", ItemName);
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，无法查询到您所输入的物品！"), Event->message.source.value());
			return;
		}
		
		QueryUrl += DataCenterName + "/" + std::to_string(ItemRowResult.fetchOne().get(0).get<int>());

		cpr::Response Response = cpr::Get(cpr::Url{ QueryUrl }, 
			cpr::Parameters{ {"listings", "10"}, {"entries", "0"}, {"hq", bQueryHQ ? "true" : "false"} }
		);

		if (Response.status_code != 200 || Response.text.empty())
		{
			Event->botlogger.warning("Query market board data failed, url : '", Response.url, "', status : ", Response.status_code);
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法提供您需要的数据！"), Event->message.source.value());
			return;
		}

		auto ResponseJson = nlohmann::json::parse(Response.text);
		if (!ResponseJson["listings"].is_array() || ResponseJson.find("listings") == ResponseJson.cend())
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，暂时没有查询到您需要的数据！"), Event->message.source.value());
			return;
		}

		long long LastReviewTime = 0;
		std::vector<std::vector<std::string>> ReplySource;
		ReplySource.push_back(std::vector<std::string>{"价格", "数量", "总价", "来源"});

		auto ListingListJson = ResponseJson["listings"];
		for (nlohmann::json ListingItem : ListingListJson)
		{
			if (LastReviewTime == 0)
			{
				LastReviewTime = ListingItem["lastReviewTime"];
			}

			long PricePerUnit = ListingItem["pricePerUnit"];
			long TotalPrice = ListingItem["total"];
			int ItemCount = ListingItem["quantity"];

			std::string WorldName = ListingItem["worldName"];
			std::string RetainerName = ListingItem["retainerName"];

			ReplySource.push_back(std::vector<std::string>{std::to_string(PricePerUnit), std::to_string(ItemCount), std::to_string(TotalPrice), WorldName});
		}

		std::string ReplyMessage = UGenericStringUtils::FormatTextTableStyle(10, ReplySource);
		ReplyMessage += "\r\n最后更新时间为 : " + UGenericStringUtils::ConvertTimestamp(LastReviewTime);
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
	mysqlx::Schema FFXIVSchema = MysqlSession.getSchema("ffxiv_data_schema");
	if (!FFXIVSchema.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法刷新大区服务器映射表！"), Event->message.source.value());
		return;
	}

	mysqlx::Table DCTable = FFXIVSchema.getTable("ffxiv_datacenter_table", true);
	if (!DCTable.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法刷新大区服务器映射表！"), Event->message.source.value());
		return;
	}

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

		mysqlx::SqlResult Result = MysqlSession.sql("truncate table ffxiv_datacenter_table").execute();
		if (Result.getWarnings().begin() != Result.getWarnings().end())
		{
			Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，刷新数据库失败，无法刷新大区服务器映射表！"), Event->message.source.value());
			return;
		}

		std::string DCName, RegionName;
		MysqlSession.startTransaction();

		for (nlohmann::json RegionItem : RegionListJson)
		{
			std::string WorldIdArrayStr;
			std::vector<int> WorldIdList = RegionItem["worlds"];
			for (const int& WorldItem : WorldIdList)
			{
				WorldIdArrayStr += std::to_string(WorldItem) + ",";
			}
			WorldIdArrayStr.erase(WorldIdArrayStr.cend());

			DCName = RegionItem["name"];
			RegionName = RegionItem["region"];

			DCTable.insert("dc_name", "dc_region", "dc_world_ids").values(DCName, RegionName, WorldIdArrayStr).execute();
		}

		MysqlSession.commit();

		Event->group.quoteAndSendMessage(MiraiCP::PlainText("大区服务器映射表成功刷新！"), Event->message.source.value());
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
	}

#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
#endif
}

void FCommandProcessor_FFXIV::ProcessCommand_RefreshItemIntro(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments)
{
	mysqlx::Schema FFXIVSchema = MysqlSession.getSchema("ffxiv_data_schema");
	if (!FFXIVSchema.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法刷新大区服务器映射表！"), Event->message.source.value());
		return;
	}

	mysqlx::Table ItemIntroTable = FFXIVSchema.getTable("ffxiv_item_intro_table", true);
	if (!ItemIntroTable.existsInDatabase())
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，数据库异常，无法刷新大区服务器映射表！"), Event->message.source.value());
		return;
	}

#if WITH_HTTP_REQUEST && WITH_OPENSSL
	try {
		std::string QueryItemUrl("https://cafemaker.wakingsands.com/item");

		int TotalQueryCount = 13;
		int QueryFailedCount = 0;
		int TotalItemCount = 0;

		MysqlSession.startTransaction();

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

				TotalItemCount++;

				int ItemId = ItemInfo["ID"];
				std::string ItemIconPath = ItemInfo["Icon"];
				std::string ItemDetailsPath = ItemInfo["Url"];

				ItemIntroTable.insert("item_id", "item_name", "item_icon_path", "item_details_path").values(ItemId, ItemName, ItemIconPath, ItemDetailsPath).execute();
			}

			int ProgressPercent = (Index + 1) / TotalQueryCount * 100;
			std::string ProgressPercentReply("当前处理物品信息列表进度..........");
			ProgressPercentReply += std::to_string(ProgressPercent) + "%";
			Event->group.sendMessage(MiraiCP::PlainText(ProgressPercentReply));
		}

		MysqlSession.commit();

		std::string ReplyMessage("物品ID映射表刷新完成！\r\n");
		ReplyMessage += " - 一共调用了" + std::to_string(TotalQueryCount) + "次接口，失败了" + std::to_string(QueryFailedCount) + "次.\r\n";
		ReplyMessage += "一共刷新了" + std::to_string(TotalItemCount) + "个物品ID。";

		Event->group.quoteAndSendMessage(MiraiCP::PlainText(ReplyMessage), Event->message.source.value());
	}
	catch (nlohmann::json::exception& Error) {
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
	}

#else
	Event->group.quoteAndSendMessage(MiraiCP::PlainText("非常抱歉，查询接口失败了，无法刷新映射表！"), Event->message.source.value());
#endif
}

void FCommandProcessor_FFXIV::ProcessCommand_DreamCraystal(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments)
{
	if (Arguments.size() < 2 || (Arguments[1].find("推车") == std::string::npos))
	{
		Event->group.quoteAndSendMessage(MiraiCP::PlainText("使用示例：\r\n1./ffxiv dream_craystal 推车\r\n2./ffxiv dream_craystal 推车黑魔"), Event->message.source.value());
		return;
	}

	std::ifstream ConfigFile("DreamCrystalConfig.json");
	if (!ConfigFile.is_open())
	{
		Event->botlogger.warning("加载DreamCrystalConfig.json失败。");
		return;
	}

	std::string ConfigContent((std::istreambuf_iterator<char>(ConfigFile)), std::istreambuf_iterator<char>());
	if (ConfigContent.empty())
	{
		Event->botlogger.warning("读取DreamCrystalConfig.json失败。");
		return;
	}

	try
	{
		nlohmann::json Json = nlohmann::json::parse(ConfigContent);

		auto FoundIt = Json.find(Arguments[1]);
		if (FoundIt == Json.end() || !FoundIt->is_array())
		{
			std::stringstream ResponseStream;
			ResponseStream << "尚未配置 "  << Arguments[1] << " 数据，请联系管理员进行添加。";
			Event->group.quoteAndSendMessage(MiraiCP::PlainText(ResponseStream.str().c_str()), Event->message.source.value());
			return;
		}

		boost::random::uniform_int_distribution<> dist(0, FoundIt->size() - 1);
		boost::random::random_device range;

		int RandomIndex = dist(range);
		std::string ContentForParse = (*FoundIt)[RandomIndex];
		std::string ParsedMessage;
		ParseDreamCraystalMessage(Json, ContentForParse, ParsedMessage);

		std::string ReplyMessage("嗯嗯？");
		ReplyMessage.append(ParsedMessage);

		Event->group.quoteAndSendMessage(MiraiCP::PlainText(ReplyMessage.c_str()), Event->message.source.value());
	}
	catch (std::exception& Error)
	{
		Event->botlogger.warning("解析DreamCrystalConfig.json失败。");
		return;
	}
}

bool FCommandProcessor_FFXIV::ParseDreamCraystalMessage(const nlohmann::json& Json, const std::string& Key, std::string& Out)
{
	std::string Result = Key;

	std::regex Pattern("\\{[\\S\\s][^\\}]+\\}");
	auto MatchIt = std::sregex_iterator(Result.cbegin(), Result.cend(), Pattern);
	auto MatchEnd = std::sregex_iterator();

	if (MatchIt == MatchEnd)
	{
		Out = Result;
		return true;
	}

	for (std::sregex_iterator It = MatchIt; It != MatchEnd; ++It)
	{
		std::string MatchedContent = It->str();
		MatchedContent.erase(MatchedContent.begin());
		if (MatchedContent.at(0) == '%')
		{
			MatchedContent.erase(MatchedContent.begin());
		}
		MatchedContent.erase(MatchedContent.end() - 1);

		auto FoundIt = Json.find(MatchedContent);
		if (FoundIt == Json.end() || !FoundIt->is_array())
		{
			std::stringstream ResponseStream;
			ResponseStream << "尚未配置 " << MatchedContent << " 数据，请联系管理员进行添加。";
			Out = ResponseStream.str();
			return false;
		}

		boost::random::uniform_int_distribution<> dist(0, FoundIt->size() - 1);
		boost::random::random_device range;

		int RandomIndex = dist(range);
		std::string ContentForParse = (*FoundIt)[RandomIndex];
		std::string ParsedMessage;

		if (!ParseDreamCraystalMessage(Json, ContentForParse, ParsedMessage))
		{
			Out = ParsedMessage;
			return false;
		}

		boost::replace_all(Result, It->str(), ParsedMessage);
	}

	Out = Result;
	return true;
}
