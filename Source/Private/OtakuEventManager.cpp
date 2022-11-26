#include "OtakuEventManager.h"

#include <chrono>
#include <sstream>

std::mutex FOtakuEventManager::EventQueueLock;

FOtakuEventManager::FOtakuEventManager()
	: bRequestedExit(false)
{

}

FOtakuEventManager::~FOtakuEventManager()
{
	RequestExit();
}

FOtakuEventManager& FOtakuEventManager::Get()
{
	static FOtakuEventManager Instance;

	return Instance;
}

void FOtakuEventManager::Initialize()
{
	MiraiCP::Event::registerEvent<MiraiCP::GroupMessageEvent>([&](MiraiCP::GroupMessageEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::GroupMessageEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::GroupInviteEvent>([&](MiraiCP::GroupInviteEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::GroupInviteEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::MemberJoinEvent>([&](MiraiCP::MemberJoinEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::MemberJoinEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::RecallEvent>([&](MiraiCP::RecallEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::RecallEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::BotJoinGroupEvent>([&](MiraiCP::BotJoinGroupEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::BotJoinGroupEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::GroupTempMessageEvent>([&](MiraiCP::GroupTempMessageEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::GroupTempMessageEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::NudgeEvent>([&](MiraiCP::NudgeEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::NudgeEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::BotLeaveEvent>([&](MiraiCP::BotLeaveEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::BotLeaveEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::MemberJoinRequestEvent>([&](MiraiCP::MemberJoinRequestEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::MemberJoinRequestEvent>(Event)); });
	MiraiCP::Event::registerEvent<MiraiCP::MemberLeaveEvent>([&](MiraiCP::MemberLeaveEvent Event) { OnNewEventReceived(std::make_shared<MiraiCP::MemberLeaveEvent>(Event)); });

	std::thread MainLoopThread(std::bind(&FOtakuEventManager::Run, this));
	MainLoopThread.detach();
}

void FOtakuEventManager::RequestExit()
{
	if (bRequestedExit)
	{
		return;
	}

	bRequestedExit = true;

	MiraiCP::Logger::logger.info("Shutdown FOtakuEventManager.");
}

void FOtakuEventManager::Tick(float Delta)
{
	std::shared_ptr<MiraiCP::MiraiCPEvent> Event = DequeueEvent();
	if (!Event)
	{
		return;
	}

	switch (Event->getEventType())
	{
	case MiraiCP::eventTypes::GroupMessageEvent:
		ProcessGroupMessage(Event);
		break;
	case MiraiCP::eventTypes::MemberJoinEvent:
		ProcessMemberJoinMessage(Event);
		break;
	case MiraiCP::eventTypes::MemberJoinRequestEvent:
		ProcessMemberJoinRequestMessage(Event);
		break;
	case MiraiCP::eventTypes::MemberLeaveEvent:
		ProcessMemberLeaveMessage(Event);
		break;
	default:
		break;
	}
}

void FOtakuEventManager::OnNewEventReceived(const std::shared_ptr<MiraiCP::MiraiCPEvent>& NewEvent)
{
	std::scoped_lock ScopedLock(EventQueueLock);

	MiraiCPEventQueue.push(NewEvent);
}

void FOtakuEventManager::RegisterMessageCommandProcessor(EMessageCommandType CommandType, const std::shared_ptr<ICommandProcessor>& Processor)
{
	MessageCommandProcessors[CommandType] = Processor;
}

void FOtakuEventManager::Run()
{
	MiraiCP::Logger::logger.info("FOtakuEventManager start running.");
	std::chrono::steady_clock::time_point PreFinishTime = std::chrono::steady_clock::now();

	while (!bRequestedExit)
	{
		std::chrono::steady_clock::duration DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - PreFinishTime);

		Tick(DeltaTime.count() / 1000.0f);

		PreFinishTime = std::chrono::steady_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

std::shared_ptr<MiraiCP::MiraiCPEvent> FOtakuEventManager::DequeueEvent()
{
	std::scoped_lock ScopedLock(EventQueueLock);

	if (MiraiCPEventQueue.empty())
	{
		return nullptr;
	}

	std::shared_ptr<MiraiCP::MiraiCPEvent> Event = MiraiCPEventQueue.front();
	MiraiCPEventQueue.pop();

	return Event;
}

void FOtakuEventManager::ProcessGroupMessage(const std::shared_ptr<MiraiCP::MiraiCPEvent>& Event)
{
	std::shared_ptr<MiraiCP::GroupMessageEvent> GroupMessageEvent = std::dynamic_pointer_cast<MiraiCP::GroupMessageEvent>(Event);
	if (!GroupMessageEvent)
	{
		return;
	}

	for (auto MessageIt = GroupMessageEvent->message.begin(); MessageIt != GroupMessageEvent->message.end(); ++MessageIt)
	{
		// Try to process message as command.
		if (ProcessMessageCommandChecked(GroupMessageEvent, *MessageIt))
		{
			continue;
		}
	}
}

void FOtakuEventManager::ProcessMemberJoinMessage(const std::shared_ptr<MiraiCP::MiraiCPEvent>& Event)
{
	std::shared_ptr<MiraiCP::MemberJoinEvent> MemberJoinEvent = std::dynamic_pointer_cast<MiraiCP::MemberJoinEvent>(Event);
	if (!MemberJoinEvent)
	{
		return;
	}

	MemberJoinEvent->group.sendMessage(MiraiCP::PlainText("欢迎 "), MemberJoinEvent->member.at(), MiraiCP::PlainText(" 来到部队群！进群请优先改群昵称先混眼熟哦。\r\n\r\n新人推荐优先了解以下内容：\r\n1、招待码↓（师徒结对，详询群主）\r\nhttps://actff1.web.sdo.com/20190315Zhaodai/index.html#/index\r\n2、新手入门攻略站↓（自强芽推荐）\r\nhttps://ff14.org/?utm_source=ffcafe&utm_medium=website&utm_campaign=navbar\r\n3、游戏中文维基：↓\r\nhttps://ff14.huijiwiki.com/wiki/%E9%A6%96%E9%A1%B5\r\n4、禁止【买金】【代练】【代打】，FF14游戏环境并不包容此类行为。\r\n5、下本请做好职业本职工作，及时更新装备！打本中遇到什么问题，推荐出本后群内求助，请不要在副本争吵浪费时间。\r\n6、支持讨论辩论，但请勿攻击他人，互相尊重。禁止键政内容。\r\n\r\n希望小伙伴顺利度过游戏前期，游戏愉快哦~"));
}

void FOtakuEventManager::ProcessMemberJoinRequestMessage(const std::shared_ptr<MiraiCP::MiraiCPEvent>& Event)
{

}

void FOtakuEventManager::ProcessMemberLeaveMessage(const std::shared_ptr<MiraiCP::MiraiCPEvent>& Event)
{
	//std::shared_ptr<MiraiCP::MemberLeaveEvent> MemberLeaveEvent = std::dynamic_pointer_cast<MiraiCP::MemberLeaveEvent>(Event);
	//if (!MemberLeaveEvent)
	//{
	//	return;
	//}

	//std::stringstream strStream;
	//strStream << "非常遗憾， " << MemberLeaveEvent->memberid << " 永远的离开了我们，祝他一路顺风，阿门。";

	//MemberLeaveEvent->group.sendMessage(MiraiCP::PlainText(strStream.str().c_str()));
}

bool FOtakuEventManager::ProcessMessageCommandChecked(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const MiraiCP::internal::Message& Message)
{
	std::string MessageTypeName = MiraiCP::SingleMessage::messageType[Message->type];
	if (MessageTypeName != "QuoteReply" && MessageTypeName != "plainText" && MessageTypeName != "at" && MessageTypeName != "atAll")
	{
		return false;
	}

	EMessageCommandType CommandType = ExtractCommand(Message->content);
	if (CommandType == EMessageCommandType::Unsupported)
	{
		return true;
	}

	if (MessageCommandProcessors.find(CommandType) == MessageCommandProcessors.cend())
	{
		Event->botlogger.info("Not found processor for command : ", Message->content);
		return true;
	}

	std::string CommandParams = Message->content;

	std::string::const_iterator CommandSplitIt = std::find(CommandParams.cbegin(), CommandParams.cend(), ' ');
	if (CommandSplitIt == CommandParams.cend())
	{
		Event->botlogger.info("Not found params from command content : ", CommandParams);

		MessageCommandProcessors[CommandType]->ProcessMessageCommand(Event, "");
	}
	else
	{
		CommandParams.erase(CommandParams.cbegin(), CommandSplitIt + 1);

		MessageCommandProcessors[CommandType]->ProcessMessageCommand(Event, CommandParams);
	}

	return true;
}

EMessageCommandType FOtakuEventManager::ExtractCommand(const std::string& Source)
{
	if (Source.empty() || Source.at(0) != '/' || Source.size() < 2)
	{
		return EMessageCommandType::Unsupported;
	}

	std::string CommandStr = Source;
	CommandStr.erase(CommandStr.cbegin());
	
	std::string::const_iterator CommandSplitIt = std::find(CommandStr.cbegin(), CommandStr.cend(), ' ');
	if (CommandSplitIt != CommandStr.cend())
	{
		CommandStr.erase(CommandSplitIt, CommandStr.cend());
	}

	auto CommandTypeIt = MessageCommandTypeNameMap.find(CommandStr);
	if (CommandTypeIt == MessageCommandTypeNameMap.cend())
	{
		return EMessageCommandType::Unsupported;
	}

	return CommandTypeIt->second;
}
