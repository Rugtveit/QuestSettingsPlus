#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"




class ModConfig 
{
    public:
        static ModInfo modInfo;

        static Configuration& getConfig() {
            static Configuration config(modInfo);
            return config;
        }

        static bool hasBool(bool &hasMember, std::string memberName);
        static float hasFloat(bool &hasMember, std::string memberName);
       

        static struct Config_t
        {
            bool    transparentWalls            = false; 
            bool    obstacleSaberSparkle        = true; 
            bool    saberClash                  = true;
            bool    fakeGlowOnWalls             = true; 
            bool    fakeGlowOnSabers            = true; 
            bool    rainbowParticles            = false;
            float   slashParticleMultiplier     = 1.0f;
            float   explosionParticleMultiplier = 1.0f;
        } Config;

        static void saveConfig();
        static bool loadConfig();
};