#include "OtakuMiraiPluginConfig.h"
#include "MiraiCP.hpp"
#include "OtakuEventManager.h"

using namespace MiraiCP;

const PluginConfig CPPPlugin::config
{
/*Plugin Id*/           "OtakuLuo.Mirai.Plugin.Otaku",
/*Plugin Name*/         "Otaku_Mirai",
/*Plugin Version*/      "v0.0.1",
/*Plugin Auth*/         "OtakuLuo",
/*Plugin Desc*/         "Just a plugin for the mirai framework."
};

class OtakuMiraiPlugin : public CPPPlugin 
{
public:
  OtakuMiraiPlugin() : CPPPlugin() {}
  ~OtakuMiraiPlugin() override = default;

  void onEnable() override 
  {
      Logger::logger.info("OtakuMiraiPlugin loaded message from plugin. version: ", OtakuMiraiPlugin_VERSION_MAJOR, ".", OtakuMiraiPlugin_VERSION_MINOR);
      FOtakuEventManager::Get().Initialize();
  }

  void onDisable() override 
  {
      FOtakuEventManager::Get().RequestExit();
  }
};

void MiraiCP::enrollPlugin() {
	MiraiCP::enrollPlugin<OtakuMiraiPlugin>();
}
