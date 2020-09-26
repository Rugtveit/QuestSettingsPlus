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
            bool    transparentWalls            = true; 
            bool    obstacleSaberSparkle        = false; 
            bool    saberClash                  = false;
            bool    fakeGlowOnWalls             = false; 
            bool    fakeGlowOnSabers            = false; 
            bool    rainbowParticles            = true;
            float   slashParticleMultiplier     = 5.0f;
            float   explosionParticleMultiplier = 5.0f;
        } Config;

        static void saveConfig();
        static bool loadConfig();
};