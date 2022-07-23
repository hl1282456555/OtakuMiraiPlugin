#include "OtakuEventManager.h"

#include <chrono>

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

		return true;
	}

	CommandParams.erase(CommandParams.cbegin(), CommandSplitIt + 1);

	MessageCommandProcessors[CommandType]->ProcessMessageCommand(Event, CommandParams);
	return true;
}

EMessageCommandType FOtakuEventManager::ExtractCommand(const std::string& Source)
{
	if (Source.empty() || Source.at(0) != '/')
	{
		return EMessageCommandType::Unsupported;
	}

	std::string CommandStr = Source;
	CommandStr.erase(CommandStr.cbegin());
	
	std::string::const_iterator CommandSplitIt = std::find(CommandStr.cbegin(), CommandStr.cend(), ' ');
	if (CommandSplitIt == CommandStr.cend())
	{
		return EMessageCommandType::Unsupported;
	}

	CommandStr.erase(CommandSplitIt, CommandStr.cend());

	auto CommandTypeIt = MessageCommandTypeNameMap.find(CommandStr);
	if (CommandTypeIt == MessageCommandTypeNameMap.cend())
	{
		return EMessageCommandType::Unsupported;
	}

	return CommandTypeIt->second;
}
