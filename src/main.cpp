//#include <unordered_set>
#include  "include/main.hpp"
#include "codegen/UnityEngine/Transform.hpp"
#include "codegen/UnityEngine/GameObject.hpp"
#include "codegen/GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "codegen/UnityEngine/Color32.hpp"
#include "codegen/UnityEngine/ParticleSystem.hpp"
#include "codegen/GlobalNamespace/NoteCutParticlesEffect.hpp"
#include "codegen/UnityEngine/ParticleSystem_MainModule.hpp"
#include "codegen/UnityEngine/Mathf.hpp"

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}
//Config
bool transparentWalls = true; 
bool fakeGlowOnWalls = false; 
bool obstacleSaberSparke = false; 

float slashParticleMultiplier = 0.5f;
float explosionParticleMultiplier = 3.0f;

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

MAKE_HOOK_OFFSETLESS(ObstacleSaberSparkleEffect_StartEmission, void, Il2CppObject* self)
{
    if(obstacleSaberSparke) ObstacleSaberSparkleEffect_StartEmission(self);
}

MAKE_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, void, GlobalNamespace::NoteCutParticlesEffect* self, Vector3 pos, Vector3 cutNormal, Vector3 saberDir, 
                    Vector3 moveVec, UnityEngine::Color32 color, int sparkleParticlesCount, int explosionParticlesCount, float lifeTimeMultiplier, int saberType)
{
    sparkleParticlesCount = UnityEngine::Mathf::FloorToInt(sparkleParticlesCount * slashParticleMultiplier); 
    explosionParticlesCount = UnityEngine::Mathf::FloorToInt(sparkleParticlesCount * explosionParticleMultiplier); 
    NoteCutParticlesEffect_SpawnParticles(self, pos, cutNormal, saberDir, moveVec, color, sparkleParticlesCount, explosionParticlesCount, lifeTimeMultiplier, saberType);
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
    INSTALL_HOOK_OFFSETLESS(ObstacleSaberSparkleEffect_StartEmission, il2cpp_utils::FindMethodUnsafe("", "ObstacleSaberSparkleEffect", "StartEmission", 0));
    INSTALL_HOOK_OFFSETLESS(NoteCutParticlesEffect_SpawnParticles, il2cpp_utils::FindMethodUnsafe("", "NoteCutParticlesEffect", "SpawnParticles", 9));
}
