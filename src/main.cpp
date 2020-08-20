//#include <unordered_set>
#include  "include/main.hpp"
#include "codegen/UnityEngine/Transform.hpp"
#include "codegen/UnityEngine/GameObject.hpp"
#include "codegen/GlobalNamespace/ParametricBoxFakeGlowController.hpp"

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

bool transparentWalls = true; 
bool fakeGlowOnWalls = false; 

MAKE_HOOK_OFFSETLESS(StretchableObstacle_SetSizeAndColor, void, GlobalNamespace::StretchableObstacle* self, float width, float height, float length, UnityEngine::Color color)
{
    auto* obstacleCore = self->obstacleCore; 
    //getLogger().info("obstacleCore: %p", obstacleCore);

    auto obstacleCoreGO = obstacleCore->get_gameObject();
    //getLogger().info("obstacleCoreGO: %p", obstacleCoreGO);

    obstacleCoreGO->SetActive(!transparentWalls);

    auto obstacleFakeGlow = self->obstacleFakeGlow;
    //getLogger().info("obstacleFakeGlow: %p", obstacleFakeGlow);

    auto obstacleFakeGlowGO = obstacleFakeGlow->get_gameObject();
    //getLogger().info("obstacleFakeGlowGO: %p", obstacleFakeGlowGO);

    obstacleFakeGlowGO->SetActive(fakeGlowOnWalls);

    //getLogger().info("Done setting fakeGlow to %d and transparentWalls to %d", fakeGlowOnWalls, transparentWalls);
    StretchableObstacle_SetSizeAndColor(self, width, height, length, color);
}


extern "C" void setup(ModInfo& info) 
{
    info.id = "QSettingsPlus";
    info.version = "0.1.0";
    modInfo = info;
    getLogger().info(info.version);
    getLogger().info(info.id);
}

extern "C" void load() 
{
    INSTALL_HOOK_OFFSETLESS(StretchableObstacle_SetSizeAndColor, il2cpp_utils::FindMethodUnsafe("", "StretchableObstacle", "SetSizeAndColor", 4));
}
