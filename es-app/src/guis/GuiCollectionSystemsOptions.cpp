#include "guis/GuiCollectionSystemsOptions.h"
#include "Settings.h"
#include "components/OptionListComponent.h"
#include "components/ComponentList.h"
#include <vector>
#include <string>

GuiCollectionSystemsOptions::GuiCollectionSystemsOptions(Window* window) : GuiSettings(window, "GAME COLLECTION SETTINGS")
{
	auto auto_collections = std::make_shared<OptionListComponent<std::string>>(mWindow, "AUTO COLLECTIONS", true);

	std::string current = Settings::getInstance()->getString("CollectionSystemsAuto");

	auto_collections->add("favorites", "favorites", current.find("favorites") != std::string::npos);
	auto_collections->add("lastplayed", "lastplayed", current.find("lastplayed") != std::string::npos);
	auto_collections->add("all", "all", current.find("all") != std::string::npos);

	addWithLabel("AUTO COLLECTIONS", auto_collections);

	addSaveFunc([auto_collections] {
		std::string selected;
		auto sels = auto_collections->getSelected();
		for(auto it = sels.begin(); it != sels.end(); it++)
		{
			if(!selected.empty()) selected += ", ";
			selected += *it;
		}
		Settings::getInstance()->setString("CollectionSystemsAuto", selected);
	});
}
