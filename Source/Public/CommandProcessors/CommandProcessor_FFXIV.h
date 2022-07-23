#ifndef COMMAND_PROCESSOR_FFXIV_H
#define COMMAND_PROCESSOR_FFXIV_H

#include "ICommandProcessor.h"
#include "OtakuMiraiStructures.h"

#include <map>
#include <memory>
#include <functional>
#include <thread>

class FCommandProcessor_FFXIV : public ICommandProcessor
{
public:
	FCommandProcessor_FFXIV();
	virtual ~FCommandProcessor_FFXIV() override;

	virtual EMessageCommandType ProcessorType() override;

	virtual void ProcessMessageCommand(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::string& Params) override;

protected:
	void BindAllCommandProcessors();

	virtual void ProcessCommand_ListWorlds(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments);
	virtual void ProcessCommand_MarketItem(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments);
	virtual void ProcessCommand_RefreshDCMap(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments);
	virtual void ProcessCommand_RefreshItemIds(const std::shared_ptr<MiraiCP::GroupMessageEvent>& Event, const std::vector<std::string>& Arguments);

private:
	std::map<EFFXIVCommandType, std::function<void(const std::shared_ptr<MiraiCP::GroupMessageEvent>&, const std::vector<std::string>&)>> CommandProcessor;

	std::map<std::string, std::string> DataCenterMap;
	std::map<std::string, int> ItemIdMap;
};

#endif