#include "CollectionSystemManager.h"
#include "SystemData.h"
#include "Settings.h"
#include "Log.h"
#include "views/ViewController.h"
#include <random>

CollectionSystemManager* CollectionSystemManager::sInstance = NULL;

CollectionSystemManager::CollectionSystemManager(Window* window) : mWindow(window) { LOG(LogInfo) << "CollectionSystemManager initialized"; }
CollectionSystemManager::~CollectionSystemManager() { LOG(LogInfo) << "CollectionSystemManager deinitialized"; }
CollectionSystemManager* CollectionSystemManager::get() { return sInstance; }

void CollectionSystemManager::init(Window* window) { if(sInstance == NULL) sInstance = new CollectionSystemManager(window); }
void CollectionSystemManager::deinit() { delete sInstance; sInstance = NULL; }

void CollectionSystemManager::loadCollectionSystems()
{
	LOG(LogInfo) << "Loading collection systems...";
	loadEnabledListFromSettings();
	updateSystemsList();
}

void CollectionSystemManager::loadEnabledListFromSettings()
{
	mAutoCollectionSystemsData.clear();
	std::string current = Settings::getInstance()->getString("CollectionSystemsAuto");

	CollectionSystemDecl decl;
	CollectionSystemData sysData;
	decl.name = "favorites"; decl.longName = "favorites"; decl.themeFolder = "auto-favorites";
	sysData.decl = decl; sysData.isEnabled = current.find("favorites") != std::string::npos; sysData.system = NULL;
	mAutoCollectionSystemsData["favorites"] = sysData;

	decl.name = "lastplayed"; decl.longName = "lastplayed"; decl.themeFolder = "auto-lastplayed";
	sysData.decl = decl; sysData.isEnabled = current.find("lastplayed") != std::string::npos; sysData.system = NULL;
	mAutoCollectionSystemsData["lastplayed"] = sysData;

	decl.name = "all"; decl.longName = "all games"; decl.themeFolder = "auto-allgames";
	sysData.decl = decl; sysData.isEnabled = current.find("all") != std::string::npos; sysData.system = NULL;
	mAutoCollectionSystemsData["all"] = sysData;
}

void CollectionSystemManager::updateSystemsList()
{
	LOG(LogInfo) << "Updating collection systems...";
	for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); )
	{
		if((*it)->isCollection()) { delete *it; it = SystemData::sSystemVector.erase(it); }
		else ++it;
	}
	for(auto it = mAutoCollectionSystemsData.begin(); it != mAutoCollectionSystemsData.end(); ++it)
	{
		if(it->second.isEnabled)
		{
			LOG(LogInfo) << "  Adding collection: " << it->second.decl.name;
			SystemEnvironmentData* envData = new SystemEnvironmentData{"", std::vector<std::string>(), "", std::vector<PlatformIds::PlatformId>()};
			SystemData* newSys = new SystemData(it->second.decl.name, it->second.decl.longName, envData, it->second.decl.themeFolder, true);
			it->second.system = newSys;
			SystemData::sSystemVector.insert(SystemData::sSystemVector.begin(), newSys);
		}
	}
	LOG(LogInfo) << "Total systems: " << SystemData::sSystemVector.size();
}

bool CollectionSystemManager::toggleGameInCollection(SystemData* file, const std::string& collectionName) { LOG(LogInfo) << "Toggle: " << collectionName; return true; }
