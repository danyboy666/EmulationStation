#include "guis/GuiCollectionSystemsOptions.h"
#include "Settings.h"
#include "CollectionSystemManager.h"
#include "views/ViewController.h"
#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"
#include <vector>
#include <string>

GuiCollectionSystemsOptions::GuiCollectionSystemsOptions(Window* window) : GuiSettings(window, "GAME COLLECTION SETTINGS")
{
	// Auto collections multi-select
	auto auto_collections = std::make_shared<OptionListComponent<std::string>>(mWindow, "AUTO COLLECTIONS", true);
	std::string current = Settings::getInstance()->getString("CollectionSystemsAuto");
	auto_collections->add("favorites", "favorites", current.find("favorites") != std::string::npos);
	auto_collections->add("lastplayed", "lastplayed", current.find("lastplayed") != std::string::npos);
	auto_collections->add("all", "all", current.find("all") != std::string::npos);
	addWithLabel("AUTO COLLECTIONS", auto_collections);

	// Group unthemed custom collections
	auto bundle = std::make_shared<SwitchComponent>(mWindow);
	bundle->setState(Settings::getInstance()->getBool("UseCustomCollectionsSystem"));
	addWithLabel("GROUP UNTHEMED CUSTOM COLLECTIONS", bundle);

	// Sort custom collections and systems
	auto sort = std::make_shared<SwitchComponent>(mWindow);
	sort->setState(Settings::getInstance()->getBool("SortAllSystems"));
	addWithLabel("SORT CUSTOM COLLECTIONS AND SYSTEMS", sort);

	// Show system name in collections
	auto showSystem = std::make_shared<SwitchComponent>(mWindow);
	showSystem->setState(Settings::getInstance()->getBool("CollectionShowSystemInfo"));
	addWithLabel("SHOW SYSTEM NAME IN COLLECTIONS", showSystem);

	// Double press Y to remove from favorites
	auto doublePress = std::make_shared<SwitchComponent>(mWindow);
	doublePress->setState(Settings::getInstance()->getBool("DoublePressRemovesFromFavs"));
	addWithLabel("PRESS (Y) TWICE TO REMOVE FROM FAVS./COLL.", doublePress);

	addSaveFunc([auto_collections, bundle, sort, showSystem, doublePress] {
		std::string selected;
		auto sels = auto_collections->getSelected();
		for(auto it = sels.begin(); it != sels.end(); it++)
		{
			if(!selected.empty()) selected += ", ";
			selected += *it;
		}
		Settings::getInstance()->setString("CollectionSystemsAuto", selected);
		Settings::getInstance()->setBool("UseCustomCollectionsSystem", bundle->getState());
		Settings::getInstance()->setBool("SortAllSystems", sort->getState());
		Settings::getInstance()->setBool("CollectionShowSystemInfo", showSystem->getState());
		Settings::getInstance()->setBool("DoublePressRemovesFromFavs", doublePress->getState());

		if(CollectionSystemManager::get())
		{
			CollectionSystemManager::get()->loadEnabledListFromSettings();
			CollectionSystemManager::get()->updateSystemsList();
		}
		ViewController::get()->reloadAll();
	});
}
