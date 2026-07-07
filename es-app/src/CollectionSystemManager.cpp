#include "CollectionSystemManager.h"
#include "SystemData.h"
#include "Log.h"

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
}

void CollectionSystemManager::updateSystemsList()
{
	LOG(LogInfo) << "Updating collection systems list...";
}

SystemData* CollectionSystemManager::getSystemToView(SystemData* sys)
{
	return sys;
}
