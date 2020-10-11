#include  "include/main.hpp"
#include "codegen/include/UnityEngine/Random.hpp"
#include <algorithm>


ModConfig::Config_t ModConfig::Config;
bool scoreSubmissionDisabled = false;

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}
/// @brief Return a random hsv color in UnityEngine Color (RGBA)
UnityEngine::Color getRandomHSV()
{
    float h = UnityEngine::Mathf::Lerp(0.0f, 1.0f, UnityEngine::Random::get_value());
    float s = UnityEngine::Mathf::Lerp(1.0f, 1.0f, UnityEngine::Random::get_value());
    float v = UnityEngine::Mathf::Lerp(1.0f, 1.0f, UnityEngine::Random::get_value());
    UnityEngine::Color result = UnityEngine::Color::HSVToRGB(h, s, v, true);
    result.a = UnityEngine::Mathf::Lerp(1.0f, 1.0f, UnityEngine::Random::get_value());
    return result;
}
/// @brief Converts Color to Color32
/// @param color UnityEngine Color input you want to convert
UnityEngine::Color32 ColorToColor32(UnityEngine::Color color)
{
    uint8_t r = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.r* 256.0f)));
    uint8_t g = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.g* 256.0f)));
    uint8_t b = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.b* 256.0f)));
    uint8_t a = UnityEngine::Mathf::Max(0, UnityEngine::Mathf::Min(255, (int)UnityEngine::Mathf::FloorToInt( color.a* 256.0f)));
    return UnityEngine::Color32(r,g,b,a);
}

MAKE_HOOK_OFFSETLESS(StretchableObstacle_SetSizeAndColor, void, GlobalNamespace::StretchableObstacle* self, float width, float height, float length, UnityEngine::Color color)
{
    // Enabling transparent walls
    // I get the obstacleCore from self (instance)
    // I get the gameObject of obstacle core and make it inactive.
    // Disabling scoresubmission to prevent users from getting accidently banned.
    // Also this is seemed as cheating on PC so this is to make it fair between platforms.
    if(ModConfig::Config.transparentWalls)
    {
        auto* obstacleCore = self->obstacleCore; 
        getLogger().info("obstacleCore: %p", obstacleCore);

        auto obstacleCoreGO = obstacleCore->get_gameObject();
        getLogger().info("obstacleCoreGO: %p", obstacleCoreGO);
        obstacleCoreGO->SetActive(false);

    }
    // Disabling fake glow on walls
    // Get the obstacle fake glow from self (instance)
    // I get the gameObject of obstacle fake glow and make it inactive.
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


MAKE_HOOK_OFFSETLESS(ObstacleSaberSparkleEffectManager_Update, void, Il2CppObject* self)
{
    // Disabling ObstacleSaberSparkle
    // Run base game method if obstacleSaberSparkle is true
    // or else do nothing
    if(ModConfig::Config.obstacleSaberSparkle) ObstacleSaberSparkleEffectManager_Update(self);
}

MAKE_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, void, GlobalNamespace::NoteCutParticlesEffect* self, Vector3 pos, Vector3 cutNormal, Vector3 saberDir, 
                    Vector3 moveVec, UnityEngine::Color32 color, int sparkleParticlesCount, int explosionParticlesCount, float lifeTimeMultiplier, int saberType)
{
    // Particles
    // Multiply default count with config multipliers
    // Floor the final value since the method expects ints
    sparkleParticlesCount = UnityEngine::Mathf::FloorToInt(sparkleParticlesCount * ModConfig::Config.slashParticleMultiplier); 
    explosionParticlesCount = UnityEngine::Mathf::FloorToInt(sparkleParticlesCount * ModConfig::Config.explosionParticleMultiplier); 
    // Rainbow particles
    // Get a random hsv color and convert it from a color to color32
    // Method expects a color32 and not color
    if(ModConfig::Config.rainbowParticles) color = ColorToColor32(getRandomHSV());
    NoteCutParticlesEffect_SpawnParticles(self, pos, cutNormal, saberDir, moveVec, color, sparkleParticlesCount, explosionParticlesCount, lifeTimeMultiplier, saberType);
    // Score submission
    // Disabling scoresubmission if particle count is over the treshold of lagging the game.
    // This lag can give users a big advantage since the game can accidently calculate the cut of a note twice
    if(scoreSubmissionDisabled) return;
    if(ModConfig::Config.slashParticleMultiplier > 10.0f || ModConfig::Config.explosionParticleMultiplier > 10.0f)
    {
        scoreSubmissionDisabled = true;
        bs_utils::Submission::disable(ModConfig::modInfo);
        getLogger().info("Disabled score submission - QSettingsPlus");
    }

}

UnityEngine::Color noGlowColor = {0.0f, 0.0f, 0.0f, 0.0f};
MAKE_HOOK_OFFSETLESS(SetSaberFakeGlowColor_SetColors, void, GlobalNamespace::SetSaberFakeGlowColor* self)
{
    // Disabling fake glow on sabers
    // Run base game method if fakeGlowOnSabers is true
    // else change parametric3SliceSprite.color to be 0 on all color channels
    // Refresh the sprites so the color gets updated for the sprites. 
    if(ModConfig::Config.fakeGlowOnSabers) return SetSaberFakeGlowColor_SetColors(self);
    auto* parametric3SliceSprite = self->parametric3SliceSprite;
    parametric3SliceSprite->color = noGlowColor;
    parametric3SliceSprite->Refresh();
}

MAKE_HOOK_OFFSETLESS(SaberClashChecker_Update, void, Il2CppObject* self)
{
    // Disabling SaberClash
    // Run base game method if saberClash is true
    // or else do nothing
    if(ModConfig::Config.saberClash) return SaberClashChecker_Update(self);
    //CRASH_UNLESS(il2cpp_utils::SetPropertyValue(self,"sabersAreClashing", false));
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
    INSTALL_HOOK_OFFSETLESS(ObstacleSaberSparkleEffectManager_Update, il2cpp_utils::FindMethodUnsafe("", "ObstacleSaberSparkleEffectManager", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, il2cpp_utils::FindMethodUnsafe("", "NoteCutParticlesEffect", "SpawnParticles", 9));
    INSTALL_HOOK_OFFSETLESS(SetSaberFakeGlowColor_SetColors, il2cpp_utils::FindMethodUnsafe("", "SetSaberFakeGlowColor", "SetColors", 0));
    INSTALL_HOOK_OFFSETLESS(SaberClashChecker_Update, il2cpp_utils::FindMethodUnsafe("", "SaberClashChecker", "Update", 0));
}
