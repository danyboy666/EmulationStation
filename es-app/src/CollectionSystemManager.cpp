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
}

void CollectionSystemManager::loadEnabledListFromSettings()
{
	mAutoCollectionSystemsData.clear();

	CollectionSystemDecl decl;
	decl.name = "favorites";
	decl.longName = "favorites";
	decl.themeFolder = "auto-favorites";
	CollectionSystemData sysData;
	sysData.decl = decl;
	sysData.isEnabled = Settings::getInstance()->getString("CollectionSystemsAuto").find("favorites") != std::string::npos;
	sysData.system = NULL;
	mAutoCollectionSystemsData["favorites"] = sysData;

	decl.name = "lastplayed";
	decl.longName = "lastplayed";
	decl.themeFolder = "auto-lastplayed";
	sysData.decl = decl;
	sysData.isEnabled = Settings::getInstance()->getString("CollectionSystemsAuto").find("lastplayed") != std::string::npos;
	sysData.system = NULL;
	mAutoCollectionSystemsData["lastplayed"] = sysData;

	decl.name = "all";
	decl.longName = "all games";
	decl.themeFolder = "auto-allgames";
	sysData.decl = decl;
	sysData.isEnabled = Settings::getInstance()->getString("CollectionSystemsAuto").find("all") != std::string::npos;
	sysData.system = NULL;
	mAutoCollectionSystemsData["all"] = sysData;
}

void CollectionSystemManager::updateSystemsList()
{
	LOG(LogInfo) << "Updating collection systems list...";
}

void CollectionSystemManager::refreshCollectionSystems()
{
	LOG(LogInfo) << "Refreshing collection systems...";
}

bool CollectionSystemManager::toggleGameInCollection(SystemData* file, const std::string& collectionName)
{
	LOG(LogInfo) << "Toggle game in collection: " << collectionName;
	return true;
}
