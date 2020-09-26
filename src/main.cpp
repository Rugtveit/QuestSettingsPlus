#include  "include/main.hpp"
#include "codegen/include/UnityEngine/Random.hpp"
#include <algorithm>


ModConfig::Config_t ModConfig::Config;

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

UnityEngine::Color getRandomHSV()
{
    float h = UnityEngine::Mathf::Lerp(0.0f, 1.0f, UnityEngine::Random::get_value());
    float s = UnityEngine::Mathf::Lerp(1.0f, 1.0f, UnityEngine::Random::get_value());
    float v = UnityEngine::Mathf::Lerp(1.0f, 1.0f, UnityEngine::Random::get_value());
    UnityEngine::Color result = UnityEngine::Color::HSVToRGB(h, s, v, true);
    result.a = UnityEngine::Mathf::Lerp(1.0f, 1.0f, UnityEngine::Random::get_value());
    return result;
}

UnityEngine::Color32 ColorToColor32(UnityEngine::Color color)
{
    uint8_t r = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.r* 256.0f)));
    uint8_t g = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.g* 256.0f)));
    uint8_t b = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.b* 256.0f)));
    uint8_t a = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.a* 256.0f)));
    return UnityEngine::Color32(r,g,b,a);
}

// Hook for transparentWall and fakeGlowOnWalls
MAKE_HOOK_OFFSETLESS(StretchableObstacle_SetSizeAndColor, void, GlobalNamespace::StretchableObstacle* self, float width, float height, float length, UnityEngine::Color color)
{
    if(ModConfig::Config.transparentWalls)
    {
        auto* obstacleCore = self->obstacleCore; 
        getLogger().info("obstacleCore: %p", obstacleCore);

        auto obstacleCoreGO = obstacleCore->get_gameObject();
        getLogger().info("obstacleCoreGO: %p", obstacleCoreGO);
        obstacleCoreGO->SetActive(false);

        bs_utils::Submission::disable(ModConfig::modInfo);
    }
    
    if(!ModConfig::Config.fakeGlowOnWalls)
    {
        auto obstacleFakeGlow = self->obstacleFakeGlow;
        getLogger().info("obstacleFakeGlow: %p", obstacleFakeGlow);

        auto obstacleFakeGlowGO = obstacleFakeGlow->get_gameObject();
        getLogger().info("obstacleFakeGlowGO: %p", obstacleFakeGlowGO);

        obstacleFakeGlowGO->SetActive(false);
    }

    getLogger().info("Done setting fakeGlow to %d and ModConfig::Config.transparentWalls to %d", ModConfig::Config.fakeGlowOnWalls, ModConfig::Config.transparentWalls);
    StretchableObstacle_SetSizeAndColor(self, width, height, length, color);
}

// Hook for obstacleSaberSparkle
MAKE_HOOK_OFFSETLESS(ObstacleSaberSparkleEffect_StartEmission, void, Il2CppObject* self)
{
    if(ModConfig::Config.obstacleSaberSparkle) ObstacleSaberSparkleEffect_StartEmission(self);
}

// Hook for slashParticle and explosionParticle multiplier
MAKE_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, void, GlobalNamespace::NoteCutParticlesEffect* self, Vector3 pos, Vector3 cutNormal, Vector3 saberDir, 
                    Vector3 moveVec, UnityEngine::Color32 color, int sparkleParticlesCount, int explosionParticlesCount, float lifeTimeMultiplier, int saberType)
{
    sparkleParticlesCount = UnityEngine::Mathf::FloorToInt(sparkleParticlesCount * ModConfig::Config.slashParticleMultiplier); 
    explosionParticlesCount = UnityEngine::Mathf::FloorToInt(sparkleParticlesCount * ModConfig::Config.explosionParticleMultiplier); 
    if(ModConfig::Config.rainbowParticles) color = ColorToColor32(getRandomHSV());


    NoteCutParticlesEffect_SpawnParticles(self, pos, cutNormal, saberDir, moveVec, color, sparkleParticlesCount, explosionParticlesCount, lifeTimeMultiplier, saberType);
}

// Hook for fakeGlowOnSabers
MAKE_HOOK_OFFSETLESS(SetSaberFakeGlowColor_SetColors, void, GlobalNamespace::SetSaberFakeGlowColor* self)
{
    if(ModConfig::Config.fakeGlowOnSabers) return SetSaberFakeGlowColor_SetColors(self);
    auto* parametric3SliceSprite = self->parametric3SliceSprite;
    UnityEngine::Color noGlowColor = {0.0f, 0.0f, 0.0f, 0.0f};
    parametric3SliceSprite->color = noGlowColor;
    parametric3SliceSprite->Refresh();
}

MAKE_HOOK_OFFSETLESS(SaberClashChecker_Update, void, Il2CppObject* self)
{
    if(ModConfig::Config.saberClash) return SaberClashChecker_Update(self);
    CRASH_UNLESS(il2cpp_utils::SetPropertyValue(self,"sabersAreClashing", false));
}

extern "C" void setup(ModInfo& info) 
{
    info.id = "QSettingsPlus";
    info.version = "1.0.0";
    modInfo = info;
    ModConfig::modInfo.id = info.id;
    ModConfig::modInfo.version = info.version;
    getLogger().info(info.version);
    getLogger().info(info.id);
}

extern "C" void load() 
{
    il2cpp_functions::Init();
    if (!ModConfig::loadConfig()) ModConfig::saveConfig();
    INSTALL_HOOK_OFFSETLESS(StretchableObstacle_SetSizeAndColor, il2cpp_utils::FindMethodUnsafe("", "StretchableObstacle", "SetSizeAndColor", 4));
    INSTALL_HOOK_OFFSETLESS(ObstacleSaberSparkleEffect_StartEmission, il2cpp_utils::FindMethodUnsafe("", "ObstacleSaberSparkleEffect", "StartEmission", 0));
    INSTALL_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, il2cpp_utils::FindMethodUnsafe("", "NoteCutParticlesEffect", "SpawnParticles", 9));
    INSTALL_HOOK_OFFSETLESS(SetSaberFakeGlowColor_SetColors, il2cpp_utils::FindMethodUnsafe("", "SetSaberFakeGlowColor", "SetColors", 0));
    INSTALL_HOOK_OFFSETLESS(SaberClashChecker_Update, il2cpp_utils::FindMethodUnsafe("", "SaberClashChecker", "Update", 0));
}
