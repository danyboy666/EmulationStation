#pragma once
#include <string>
#include <vector>
#include <map>
class Window;
class SystemData;
struct CollectionSystemDecl { std::string name; std::string longName; std::string themeFolder; };
struct CollectionSystemData { SystemData* system; CollectionSystemDecl decl; bool isEnabled; };
class CollectionSystemManager
{
public:
	static CollectionSystemManager* get();
	static void init(Window* window);
	static void deinit();
	void loadCollectionSystems();
	void loadEnabledListFromSettings();
	void updateSystemsList();
	inline std::map<std::string, CollectionSystemData>& getAutoCollectionSystems() { return mAutoCollectionSystemsData; }
	inline std::map<std::string, CollectionSystemData> getCustomCollectionSystems() { return mCustomCollectionSystemsData; }
	bool isEditing() { return false; }
	std::string getEditingCollection() { return ""; }
	void exitEditMode() {}
	void setEditMode(const std::string& name) {}
	std::vector<std::string> getUnusedSystemsFromTheme() { return std::vector<std::string>(); }
	SystemData* addNewCustomCollection(const std::string& name, bool needSave = true) { return nullptr; }
	bool saveCustomCollection(SystemData* sys) { return false; }
	std::string getValidNewCollectionName(const std::string& name, int index = 0) { return name; }
	bool toggleGameInCollection(SystemData* file, const std::string& collectionName = "favorites");
private:
	CollectionSystemManager(Window* window);
	~CollectionSystemManager();
	static CollectionSystemManager* sInstance;
	Window* mWindow;
	std::map<std::string, CollectionSystemData> mAutoCollectionSystemsData;
	std::map<std::string, CollectionSystemData> mCustomCollectionSystemsData;
};
