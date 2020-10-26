#include "../include/config.hpp"
#include <string_view>


ModInfo ModConfig::modInfo;


#define GET(obj, fieldName, method) auto itr = obj.FindMember(fieldName.data()); \
if (itr == obj.MemberEnd()) { \
    return std::nullopt; \
} \
return itr->value.method(); \

std::optional<float> getFloat(rapidjson::Value& obj, std::string_view fieldName) {
    GET(obj, fieldName, GetFloat);
}

std::optional<bool> getBool(rapidjson::Value& obj, std::string_view fieldName) {
    GET(obj, fieldName, GetBool);
}

bool ModConfig::loadConfig()
{
    getConfig().Load();	
    ConfigDocument& configDoc = getConfig().config;
    if(auto TW = getBool(configDoc, "TransparentWalls")) Config.transparentWalls = *TW; else return false;
    if(auto FGOW = getBool(configDoc, "FakeGlowOnWalls")) Config.fakeGlowOnWalls = *FGOW; else return false;
    if(auto FGOS = getBool(configDoc, "FakeGlowOnSabers")) Config.fakeGlowOnSabers = *FGOS; else return false;
    if(auto OSS = getBool(configDoc, "ObstacleSaberSparkle")) Config.obstacleSaberSparkle = *OSS; else return false;
    if(auto SC = getBool(configDoc, "SaberClash")) Config.saberClash = *SC; else return false;
    if(auto RP = getBool(configDoc, "RainbowParticles")) Config.rainbowParticles = *RP; else return false;
    if(auto SPM = getFloat(configDoc, "SlashParticleMultiplier")) Config.slashParticleMultiplier = *SPM; else return false;
    if(auto SPL = getFloat(configDoc, "SlashParticleLifetimeMultiplier")) Config.slashParticleLifetime = *SPL; else return false;
    if(auto EPM = getFloat(configDoc, "ExplosionParticleMultiplier")) Config.explosionParticleMultiplier = *EPM; else return false;
    if(auto EPL = getFloat(configDoc, "ExplosionParticleLifetimeMultiplier")) Config.explosionParticleLifetime = *EPL; else return false;
	return true;

};

void ModConfig::saveConfig()
{
    ConfigDocument& configDoc = getConfig().config;
    configDoc.RemoveAllMembers();
	configDoc.SetObject();
	auto &allocator = configDoc.GetAllocator();
    configDoc.AddMember("TransparentWalls", Config.transparentWalls, allocator);
	configDoc.AddMember("FakeGlowOnWalls", Config.fakeGlowOnWalls, allocator);
	configDoc.AddMember("FakeGlowOnSabers", Config.fakeGlowOnSabers, allocator);
    configDoc.AddMember("ObstacleSaberSparkle", Config.obstacleSaberSparkle, allocator);
    configDoc.AddMember("SaberClash", Config.saberClash, allocator);
    configDoc.AddMember("RainbowParticles", Config.rainbowParticles, allocator);
	configDoc.AddMember("SlashParticleMultiplier", Config.slashParticleMultiplier, allocator);
    configDoc.AddMember("SlashParticleLifetimeMultiplier", Config.slashParticleLifetime, allocator);
    configDoc.AddMember("ExplosionParticleMultiplier", Config.explosionParticleMultiplier, allocator);
    configDoc.AddMember("ExplosionParticleLifetimeMultiplier", Config.explosionParticleLifetime, allocator);
	getConfig().Write();
}

