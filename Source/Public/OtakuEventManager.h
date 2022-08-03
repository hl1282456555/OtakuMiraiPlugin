#ifndef OTAKUEVENTMANAGER_H
#define OTAKUEVENTMANAGER_H

#include "OtakuMiraiStructures.h"
#include "ICommandProcessor.h"

#include <MiraiCP.hpp>

#include <memory>
#include <mutex>
#include <queue>
#include <map>
#include <atomic>

class FOtakuEventManager
{
public:
	FOtakuEventManager();
	virtual ~FOtakuEventManager();
	FOtakuEventManager& operator=(const FOtakuEventManager&) = delete;
	FOtakuEventManager& operator=(FOtakuEventManager&&) = delete;

public:
	static FOtakuEventManager& Get();

	virtual void Initialize();

	virtual void RequestExit();

	virtual void Tick(float Delta);

	virtual void OnNewEventReceived(const std::shared_ptr<MiraiCP::MiraiCPEvent>& NewEvent);

	virtual void RegisterMessageCommandProcessor(EMessageCommandType CommandType, const std::shared_ptr<ICommandProcessor>& Processor);

protected:
	virtual void Run();

	virtual std::shared_ptr<MiraiCP::MiraiCPEvent> DequeueEvent();

	virtual void ProcessGroupMessage(const std::shared_ptr<MiraiCP::MiraiCPEvent>& Event);

	virtual bool ProcessMessageCommandChecked(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const MiraiCP::internal::Message& Message);
	virtual EMessageCommandType ExtractCommand(const std::string& Source);

private:
	std::atomic<bool> bRequestedExit;

	static std::mutex EventQueueLock;
	std::queue<std::shared_ptr<MiraiCP::MiraiCPEvent>> MiraiCPEventQueue;

	std::map<EMessageCommandType, std::shared_ptr<ICommandProcessor>> MessageCommandProcessors;
};

template<class T>
class FProcessorAutoRegistor
{
public:
	FProcessorAutoRegistor()
	{
		if (std::is_base_of_v<ICommandProcessor, T>)
		{
			std::shared_ptr<ICommandProcessor> ProcessorInstance = std::make_shared<T>();
			FOtakuEventManager::Get().RegisterMessageCommandProcessor(ProcessorInstance->ProcessorType(), ProcessorInstance);
		}
		else
		{
			MiraiCP::Logger::logger.warning("Register a class not child of ICommandProcessor is not a supported action!");
		}
	}
};

#define REGISTER_COMMAND_PROCESSOR(Name) \
static FProcessorAutoRegistor<FCommandProcessor_##Name> Proccessor_##Name_Registor;

#endif