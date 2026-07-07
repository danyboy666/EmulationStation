#include "EmulationStation.h"
#include "guis/GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiCollectionSystemsOptions.h"
#include "views/ViewController.h"
#include "SystemData.h"

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "VolumeControl.h"
#include "scrapers/GamesDBScraper.h"
#include "scrapers/ScreenScraperScraper.h"
#include "scrapers/TheArchiveScraper.h"
#include "guis/GuiTextEditPopup.h"

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU"), mVersion(window)
{
	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };

	addEntry("SCRAPER", 0x777777FF, true, [this, openScrapeNow] {
		auto s = new GuiSettings(mWindow, "SCRAPER");
		auto scraper_list = std::make_shared<OptionListComponent<std::string>>(mWindow, "SCRAPE FROM", false);
		std::vector<std::string> scrapers = getScraperList();
		for(auto it = scrapers.begin(); it != scrapers.end(); it++)
			scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));
		s->addWithLabel("SCRAPE FROM", scraper_list);
		s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });
		auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
		scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
		s->addWithLabel("SCRAPE RATINGS", scrape_ratings);
		s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });
		ComponentListRow row;
		std::function<void()> openAndSave = openScrapeNow;
		openAndSave = [s, openAndSave] { s->save(); openAndSave(); };
		row.makeAcceptInputHandler(openAndSave);
		row.addElement(std::make_shared<TextComponent>(mWindow, "SCRAPE NOW", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.addElement(makeArrow(mWindow), false);
		s->addRow(row);
		mWindow->pushGui(s);
	});

	addEntry("SOUND SETTINGS", 0x777777FF, true, [this] {
		auto s = new GuiSettings(mWindow, "SOUND SETTINGS");
		auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
		volume->setValue((float)VolumeControl::getInstance()->getVolume());
		s->addWithLabel("SYSTEM VOLUME", volume);
		s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)std::round(volume->getValue())); });
		auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
		sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
		s->addWithLabel("ENABLE SOUNDS", sounds_enabled);
		s->addSaveFunc([sounds_enabled] { Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); });
		mWindow->pushGui(s);
	});

	addEntry("UI SETTINGS", 0x777777FF, true, [this] {
		auto s = new GuiSettings(mWindow, "UI SETTINGS");
		auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
		screensaver_time->setValue((float)(Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
		s->addWithLabel("SCREENSAVER AFTER", screensaver_time);
		s->addSaveFunc([screensaver_time] { Settings::getInstance()->setInt("ScreenSaverTime", (int)std::round(screensaver_time->getValue()) * (1000 * 60)); });
		auto screensaver_behavior = std::make_shared<OptionListComponent<std::string>>(mWindow, "SCREENSAVER BEHAVIOR", false);
		std::vector<std::string> screensavers = {"dim", "black"};
		for(auto it = screensavers.begin(); it != screensavers.end(); it++)
			screensaver_behavior->add(*it, *it, Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
		s->addWithLabel("SCREENSAVER BEHAVIOR", screensaver_behavior);
		s->addSaveFunc([screensaver_behavior] { Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected()); });
		auto framerate = std::make_shared<SwitchComponent>(mWindow);
		framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
		s->addWithLabel("SHOW FRAMERATE", framerate);
		s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });
		auto show_help = std::make_shared<SwitchComponent>(mWindow);
		show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
		s->addWithLabel("ON-SCREEN HELP", show_help);
		s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });
		auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
		quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
		s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
		s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });
		auto transition_style = std::make_shared<OptionListComponent<std::string>>(mWindow, "TRANSITION STYLE", false);
		std::vector<std::string> transitions = {"fade", "slide", "instant"};
		for(auto it = transitions.begin(); it != transitions.end(); it++)
			transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
		s->addWithLabel("TRANSITION STYLE", transition_style);
		s->addSaveFunc([transition_style] { Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected()); });
		auto gamelist_style = std::make_shared<OptionListComponent<std::string>>(mWindow, "GAMELIST VIEW STYLE", false);
		std::vector<std::string> styles = {"automatic", "basic", "detailed", "grid"};
		for(auto it = styles.begin(); it != styles.end(); it++)
			gamelist_style->add(*it, *it, Settings::getInstance()->getString("GamelistViewStyle") == *it);
		s->addWithLabel("GAMELIST VIEW STYLE", gamelist_style);
		s->addSaveFunc([gamelist_style] { Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected()); ViewController::get()->reloadAll(); });
		auto themeSets = ThemeData::getThemeSets();
		if(!themeSets.empty()) {
			auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
			if(selectedSet == themeSets.end()) selectedSet = themeSets.begin();
			auto theme_set = std::make_shared<OptionListComponent<std::string>>(mWindow, "THEME SET", false);
			for(auto it = themeSets.begin(); it != themeSets.end(); it++)
				theme_set->add(it->first, it->first, it == selectedSet);
			s->addWithLabel("THEME SET", theme_set);
			s->addSaveFunc([theme_set] { Settings::getInstance()->setString("ThemeSet", theme_set->getSelected()); ViewController::get()->reloadAll(); });
		}
		mWindow->pushGui(s);
	});

	addEntry("GAME COLLECTION SETTINGS", 0x777777FF, true, [this] {
		mWindow->pushGui(new GuiCollectionSystemsOptions(mWindow));
	});

	addEntry("OTHER SETTINGS", 0x777777FF, true, [this] {
		auto s = new GuiSettings(mWindow, "OTHER SETTINGS");
		auto max_vram = std::make_shared<SliderComponent>(mWindow, 0.f, 1000.f, 10.f, "Mb");
		max_vram->setValue((float)(Settings::getInstance()->getInt("MaxVRAM")));
		s->addWithLabel("VRAM LIMIT", max_vram);
		s->addSaveFunc([max_vram] { Settings::getInstance()->setInt("MaxVRAM", (int)std::round(max_vram->getValue())); });
		auto power_saver = std::make_shared<OptionListComponent<std::string>>(mWindow, "POWER SAVER MODES", false);
		std::vector<std::string> modes = {"disabled", "default", "enhanced", "instant"};
		for(auto it = modes.begin(); it != modes.end(); it++)
			power_saver->add(*it, *it, Settings::getInstance()->getString("PowerSaverMode") == *it);
		s->addWithLabel("POWER SAVER MODES", power_saver);
		s->addSaveFunc([power_saver] { Settings::getInstance()->setString("PowerSaverMode", power_saver->getSelected()); });
		auto gamelistsSaveMode = std::make_shared<OptionListComponent<std::string>>(mWindow, "SAVE METADATA", false);
		std::vector<std::string> saveModes = {"on exit", "always", "never"};
		for(auto it = saveModes.begin(); it != saveModes.end(); it++)
			gamelistsSaveMode->add(*it, *it, Settings::getInstance()->getString("SaveGamelistsMode") == *it);
		s->addWithLabel("SAVE METADATA", gamelistsSaveMode);
		s->addSaveFunc([gamelistsSaveMode] { Settings::getInstance()->setString("SaveGamelistsMode", gamelistsSaveMode->getSelected()); });
		auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
		parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
		s->addWithLabel("PARSE GAMESLISTS ONLY", parse_gamelists);
		s->addSaveFunc([parse_gamelists] { Settings::getInstance()->setBool("ParseGamelistOnly", parse_gamelists->getState()); });
		auto local_art = std::make_shared<SwitchComponent>(mWindow);
		local_art->setState(Settings::getInstance()->getBool("LocalArt"));
		s->addWithLabel("SEARCH FOR LOCAL ART", local_art);
		s->addSaveFunc([local_art] { Settings::getInstance()->setBool("LocalArt", local_art->getState()); });
		auto hidden_files = std::make_shared<SwitchComponent>(mWindow);
		hidden_files->setState(Settings::getInstance()->getBool("ShowHiddenFiles"));
		s->addWithLabel("SHOW HIDDEN FILES", hidden_files);
		s->addSaveFunc([hidden_files] { Settings::getInstance()->setBool("ShowHiddenFiles", hidden_files->getState()); });
		mWindow->pushGui(s);
	});

	addEntry("CONFIGURE INPUT", 0x777777FF, true, [this] { mWindow->pushGui(new GuiDetectDevice(mWindow, false, nullptr)); });

	addEntry("QUIT", 0x777777FF, true, [this] {
		auto s = new GuiSettings(mWindow, "QUIT");
		Window* window = mWindow;
		ComponentListRow row;
		row.makeAcceptInputHandler([window] { window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES", [] { runShutdownCommand(); }, "NO", nullptr)); });
		row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		s->addRow(row);
		row.elements.clear();
		row.makeAcceptInputHandler([window] { window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES", [] { runRestartCommand(); }, "NO", nullptr)); });
		row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		s->addRow(row);
		if(Settings::getInstance()->getBool("ShowExit")) {
			row.elements.clear();
			row.makeAcceptInputHandler([window] { SDL_Event ev; ev.type = SDL_QUIT; SDL_PushEvent(&ev); });
			row.addElement(std::make_shared<TextComponent>(window, "QUIT EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);
		}
		mWindow->pushGui(s);
	});

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0xC6C6C6FF);
	mVersion.setText("EMULATIONSTATION V" + strToUpper(PROGRAM_VERSION_STRING));
	mVersion.setAlignment(ALIGN_CENTER);
	addChild(&mMenu);
	addChild(&mVersion);
	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::onSizeChanged() { mVersion.setSize(mSize.x(), 0); mVersion.setPosition(0, mSize.y() - mVersion.getSize().y()); }

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, Font::get(FONT_SIZE_MEDIUM), color), true);
	if(add_arrow) row.addElement(makeArrow(mWindow), false);
	row.makeAcceptInputHandler(func);
	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input)) return true;
	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0) { delete this; return true; }
	return false;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
