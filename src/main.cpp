#include  "include/main.hpp"
#include <algorithm>
#include <cmath>


ModConfig::Config_t ModConfig::Config;
bool scoreSubmissionDisabled = false;

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

/// @brief Return a random hsv color in UnityEngine Color (RGBA)
Color getRandomHSV()
{
    RET_DEFAULT_UNLESS(il2cpp_utils::RunMethod("UnityEngine", "Random", "InitState", 582));
    float randomFloat1 = RET_DEFAULT_UNLESS(il2cpp_utils::RunMethod<float>("UnityEngine", "Random", "get_value"));
    //float randomFloat2 = RET_DEFAULT_UNLESS(il2cpp_utils::RunMethod<float>("UnityEngine", "Random", "get_value"));
    //float randomFloat3 = RET_DEFAULT_UNLESS(il2cpp_utils::RunMethod<float>("UnityEngine", "Random", "get_value"));
    //float randomFloat4 = RET_DEFAULT_UNLESS(il2cpp_utils::RunMethod<float>("UnityEngine", "Random", "get_value"));
    float h = std::lerp(0.0f, 1.0f, randomFloat1);
    float s = std::lerp(1.0f, 1.0f, randomFloat1);
    float v = std::lerp(1.0f, 1.0f, randomFloat1);
    Color result = RET_DEFAULT_UNLESS(il2cpp_utils::RunMethod<Color>("UnityEngine", "Color", "HSVToRGB", h, s, v, true));
    result.a = std::lerp(1.0f, 1.0f, randomFloat1);
    return result;
}
/// @brief Converts Color to Color32
/// @param color UnityEngine Color input you want to convert
Il2CppObject* ColorToColor32(Color color)
{
    uint8_t r = std::max(0, std::min(255, (int)std::floor(color.r*256.0f)));
    uint8_t g = std::max(0, std::min(255, (int)std::floor(color.g*256.0f)));
    uint8_t b = std::max(0, std::min(255, (int)std::floor(color.b*256.0f)));
    uint8_t a = std::max(0, std::min(255, (int)std::floor(color.a*256.0f)));
    return RET_DEFAULT_UNLESS(il2cpp_utils::New("UnityEngine", "Color32", r, g, b, a));
}

MAKE_HOOK_OFFSETLESS(StretchableObstacle_SetSizeAndColor, void, Il2CppObject* self, float width, float height, float length, Color color)
{
    // Enabling transparent walls
    // I get the obstacleCore from self (instance)
    // I get the gameObject of obstacle core and make it inactive.
    // Disabling scoresubmission to prevent users from getting accidently banned.
    // Also this is seemed as cheating on PC so this is to make it fair between platforms.
    if(ModConfig::Config.transparentWalls)
    {
        Il2CppObject* obstacleCore = RET_V_UNLESS(il2cpp_utils::GetFieldValue(self, "_obstacleCore"));
        Il2CppObject* obstacleCoreGO = RET_V_UNLESS(il2cpp_utils::GetPropertyValue(obstacleCore, "gameObject"));
        RET_V_UNLESS(il2cpp_utils::RunMethod(obstacleCoreGO, "SetActive", false));

    }
    // Disabling fake glow on walls
    // Get the obstacle fake glow from self (instance)
    // I get the gameObject of obstacle fake glow and make it inactive.
    if(!ModConfig::Config.fakeGlowOnWalls)
    {
        Il2CppObject* _obstacleFakeGlow  = RET_V_UNLESS(il2cpp_utils::GetFieldValue(self, "_obstacleCore"));
        auto obstacleFakeGlowGO = RET_V_UNLESS(il2cpp_utils::GetPropertyValue(_obstacleFakeGlow, "gameObject"));
        getLogger().info("obstacleFakeGlowGO: %p", obstacleFakeGlowGO);

        RET_V_UNLESS(il2cpp_utils::RunMethod(obstacleFakeGlowGO, "SetActive", false));
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

void SetMaxParticlesAndLifetime(Il2CppObject* particleSystem, float multiplier)
{
    if (!particleSystem) return; 
    Il2CppObject* particleModule = RET_V_UNLESS(il2cpp_utils::GetPropertyValue(particleSystem, "main"));
    RET_V_UNLESS(il2cpp_utils::SetPropertyValue(particleModule, "maxParticles", INT_MAX));
    RET_V_UNLESS(il2cpp_utils::SetPropertyValue(particleModule, "startLifetimeMultiplier", multiplier));
}




MAKE_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, void, Il2CppObject* self, Vector3 pos, Vector3 cutNormal, Vector3 saberDir, 
                    Vector3 moveVec, Il2CppObject* color, int sparkleParticlesCount, int explosionParticlesCount, float lifeTimeMultiplier)
{
    // Particles
    // Multiply default count with config multipliers
    // Floor the final value since the method expects ints
    sparkleParticlesCount = (int)std::floor(sparkleParticlesCount * ModConfig::Config.slashParticleMultiplier); 
    explosionParticlesCount = (int)std::floor(sparkleParticlesCount * ModConfig::Config.explosionParticleMultiplier); 

    Il2CppObject* sparklePS = RET_V_UNLESS(il2cpp_utils::GetFieldValue(self, "_sparklesPS"));
    SetMaxParticlesAndLifetime(sparklePS, ModConfig::Config.slashParticleMultiplier);

    Il2CppObject* explosionPS = RET_V_UNLESS(il2cpp_utils::GetFieldValue(self, "_explosionPS"));
    SetMaxParticlesAndLifetime(explosionPS, ModConfig::Config.explosionParticleLifetime);


    // Rainbow particles
    // Get a random hsv color and convert it from a color to color32
    // Method expects a color32 and not color
    if(ModConfig::Config.rainbowParticles) color = ColorToColor32(getRandomHSV());
    NoteCutParticlesEffect_SpawnParticles(self, pos, cutNormal, saberDir, moveVec, color, sparkleParticlesCount, explosionParticlesCount, lifeTimeMultiplier);
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

Color noGlowColor = {0.0f, 0.0f, 0.0f, 0.0f};
MAKE_HOOK_OFFSETLESS(SetSaberFakeGlowColor_SetColors, void, Il2CppObject* self)
{
    // Disabling fake glow on sabers
    // Run base game method if fakeGlowOnSabers is true
    // else change parametric3SliceSprite.color to be 0 on all color channels
    // Refresh the sprites so the color gets updated for the sprites. 
    if(ModConfig::Config.fakeGlowOnSabers) return SetSaberFakeGlowColor_SetColors(self);
    Il2CppObject* parametric3SliceSprite = RET_V_UNLESS(il2cpp_utils::GetFieldValue(self, "_parametric3SliceSprite"));
    RET_V_UNLESS(il2cpp_utils::SetFieldValue(parametric3SliceSprite, "color", noGlowColor));
    RET_V_UNLESS(il2cpp_utils::RunMethod(parametric3SliceSprite, "Refresh"));
}

MAKE_HOOK_OFFSETLESS(SaberClashEffect_LateUpdate, void, Il2CppObject* self)
{
    // Disabling SaberClash
    // Run base game method if saberClash is true
    // or else do nothing
    if(ModConfig::Config.saberClash) return SaberClashEffect_LateUpdate(self);
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
    INSTALL_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, il2cpp_utils::FindMethodUnsafe("", "NoteCutParticlesEffect", "SpawnParticles", 8));
    INSTALL_HOOK_OFFSETLESS(SetSaberFakeGlowColor_SetColors, il2cpp_utils::FindMethodUnsafe("", "SetSaberFakeGlowColor", "SetColors", 0));
    INSTALL_HOOK_OFFSETLESS(SaberClashEffect_LateUpdate, il2cpp_utils::FindMethodUnsafe("", "SaberClashEffect", "LateUpdate", 0));
}
