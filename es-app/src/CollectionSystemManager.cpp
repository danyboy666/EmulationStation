#include "CollectionSystemManager.h"
#include "SystemData.h"
#include "Settings.h"
#include "Log.h"
#include "views/ViewController.h"

CollectionSystemManager* CollectionSystemManager::sInstance = NULL;

CollectionSystemManager::CollectionSystemManager(Window* window) : mWindow(window)
{
	LOG(LogInfo) << "CollectionSystemManager initialized";
}

CollectionSystemManager::~CollectionSystemManager()
{
	LOG(LogInfo) << "CollectionSystemManager deinitialized";
}

CollectionSystemManager* CollectionSystemManager::get() { return sInstance; }

void CollectionSystemManager::init(Window* window)
{
	if(sInstance == NULL)
		sInstance = new CollectionSystemManager(window);
}

void CollectionSystemManager::deinit()
{
	delete sInstance;
	sInstance = NULL;
}

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
	decl.name = "favorites";
	decl.longName = "favorites";
	decl.themeFolder = "auto-favorites";
	CollectionSystemData sysData;
	sysData.decl = decl;
	sysData.isEnabled = current.find("favorites") != std::string::npos;
	sysData.system = NULL;
	mAutoCollectionSystemsData["favorites"] = sysData;

	decl.name = "lastplayed";
	decl.longName = "lastplayed";
	decl.themeFolder = "auto-lastplayed";
	sysData.decl = decl;
	sysData.isEnabled = current.find("lastplayed") != std::string::npos;
	sysData.system = NULL;
	mAutoCollectionSystemsData["lastplayed"] = sysData;

	decl.name = "all";
	decl.longName = "all games";
	decl.themeFolder = "auto-allgames";
	sysData.decl = decl;
	sysData.isEnabled = current.find("all") != std::string::npos;
	sysData.system = NULL;
	mAutoCollectionSystemsData["all"] = sysData;
}

void CollectionSystemManager::updateSystemsList()
{
	LOG(LogInfo) << "Updating collection systems list...";

	// Remove existing collection systems from the vector
	for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); )
	{
		if((*it)->isCollection())
		{
			delete *it;
			it = SystemData::sSystemVector.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Add enabled collection systems to the front of the vector
	for(auto it = mAutoCollectionSystemsData.begin(); it != mAutoCollectionSystemsData.end(); ++it)
	{
		if(it->second.isEnabled)
		{
			LOG(LogInfo) << "  Adding collection system: " << it->second.decl.name;
			// Create a virtual system for this collection
			SystemEnvironmentData* envData = new SystemEnvironmentData{"", std::vector<std::string>(), "", std::vector<PlatformIds::PlatformId>()};
			SystemData* newSys = new SystemData(it->second.decl.name, it->second.decl.longName, envData, it->second.decl.themeFolder, true);
			it->second.system = newSys;
			SystemData::sSystemVector.insert(SystemData::sSystemVector.begin(), newSys);
		}
	}

	LOG(LogInfo) << "Collection systems updated. Total systems: " << SystemData::sSystemVector.size();
}

bool CollectionSystemManager::toggleGameInCollection(SystemData* file, const std::string& collectionName)
{
	LOG(LogInfo) << "Toggle game in collection: " << collectionName;
	return true;
}
