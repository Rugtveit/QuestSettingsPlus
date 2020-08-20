//#include <unordered_set>
#include  "include/main.hpp"

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}


extern "C" void setup(ModInfo& info) 
{
    info.id = "QSettingsPlus";
    info.version = "0.1.0";
    modInfo = info;
    saberMod.modInfo = info;
    getLogger().info(info.version);
    getLogger().info(info.id);
}

extern "C" void load() 
{

}
