// MiraiCP依赖文件(只需要引入这一个)
#include <MiraiCP.hpp>
using namespace MiraiCP;

const PluginConfig CPPPlugin::config{
        "OtakuLuo.Mirai.Plugin.Otaku",          // 插件id
        "Otaku_Mirai",        // 插件名称
        "v0.0.1",            // 插件版本
        "OtakuLuo",        // 插件作者
        "Just a plugin for the mirai framework."  // 可选：插件描述
        ""
};

// 插件实例
class Main : public CPPPlugin {
public:
  // 配置插件信息
  Main() : CPPPlugin() {}
  ~Main() override = default;

  // 入口函数
  void onEnable() override {
    // 请在此处监听
      Logger::logger.info("OtakuMiraiPlugin loaded message from plugin.");
      Event::registerEvent<GroupMessageEvent>(
          [](GroupMessageEvent Event) 
          {
              Logger::logger.info("Group mesage ====> ");
          });
  }

  // 退出函数
  void onDisable() override {
    /*插件结束前执行*/
  }
};

// 绑定当前插件实例
void MiraiCP::enrollPlugin() {
  MiraiCP::enrollPlugin(new Main);
}
