#pragma once
#include <string>
#include <map>

class Window;
class SystemData;

class CollectionSystemManager
{
public:
	static CollectionSystemManager* get();
	static void init(Window* window);
	static void deinit();
	void loadCollectionSystems();
	void updateSystemsList();
	SystemData* getSystemToView(SystemData* sys);

private:
	CollectionSystemManager(Window* window);
	~CollectionSystemManager();
	static CollectionSystemManager* sInstance;
	Window* mWindow;
};
