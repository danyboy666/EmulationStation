#include "SystemData.h"
#include "Gamelist.h"
#include "CollectionSystemManager.h"
#include "FileFilterIndex.h"
#include "FileSorts.h"
#include "Log.h"
#include "Settings.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>
#include <SDL_joystick.h>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"
#include "InputManager.h"
#include <iostream>

std::vector<SystemData*> SystemData::sSystemVector;
namespace fs = boost::filesystem;

SystemData::SystemData(const std::string& name, const std::string& fullName, SystemEnvironmentData* envData, const std::string& themeFolder, bool CollectionSystem)
{
	mName = name;
	mFullName = fullName;
	mEnvData = envData;
	mThemeFolder = themeFolder;
	mIsCollectionSystem = CollectionSystem;
	mIsGameSystem = true;
	mFilterIndex = NULL;

	mRootFolder = new FileData(FOLDER, envData->mStartPath, this);
	mRootFolder->metadata.set("name", mFullName);

	if(!CollectionSystem)
	{
		if(!Settings::getInstance()->getBool("ParseGamelistOnly"))
			populateFolder(mRootFolder);
		if(!Settings::getInstance()->getBool("IgnoreGamelist"))
			parseGamelist(this);
		mRootFolder->sort(FileSorts::SortTypes.at(0));
	}

	loadTheme();
	setIsGameSystemStatus();
}

SystemData::~SystemData()
{
	if(!mIsCollectionSystem)
	{
		if(!Settings::getInstance()->getBool("IgnoreGamelist"))
			updateGamelist(this);
	}
	delete mRootFolder;
	delete mFilterIndex;
}

std::string strreplace(std::string str, const std::string& replace, const std::string& with)
{
	size_t pos;
	while((pos = str.find(replace)) != std::string::npos)
		str = str.replace(pos, replace.length(), with.c_str(), with.length());
	return str;
}

std::string escapePath(const boost::filesystem::path& path)
{
	std::string pathStr = path.string();
	const char* invalidChars = " '\"\\!$^&*(){}[]?;<>";
	for(unsigned int i = 0; i < pathStr.length(); i++)
	{
		char c;
		unsigned int charNum = 0;
		do {
			c = invalidChars[charNum];
			if(pathStr[i] == c) { pathStr.insert(i, "\\"); i++; break; }
			charNum++;
		} while(c != '\0');
	}
	return pathStr;
}

void SystemData::launchGame(Window* window, FileData* game)
{
	LOG(LogInfo) << "Attempting to launch game...";
	AudioManager::getInstance()->deinit();
	VolumeControl::getInstance()->deinit();
	window->deinit();

	std::string command = mEnvData->mLaunchCommand;
	const std::string rom = escapePath(game->getPath());
	const std::string basename = game->getPath().stem().string();
	const std::string rom_raw = fs::path(game->getPath()).make_preferred().string();

	command = strreplace(command, "%ROM%", rom);
	command = strreplace(command, "%BASENAME%", basename);
	command = strreplace(command, "%ROM_RAW%", rom_raw);

	LOG(LogInfo) << "\t" << command;
	std::cout << "==============================================\n";
	int exitCode = runSystemCommand(command);
	std::cout << "==============================================\n";

	if(exitCode != 0)
		LOG(LogWarning) << "...launch terminated with nonzero exit code " << exitCode << "!";

	window->init();
	VolumeControl::getInstance()->init();
	AudioManager::getInstance()->init();
	window->normalizeNextUpdate();

	int timesPlayed = game->metadata.getInt("playcount") + 1;
	game->metadata.set("playcount", std::to_string(static_cast<long long>(timesPlayed)));
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	game->metadata.setTime("lastplayed", time);
}

void SystemData::populateFolder(FileData* folder)
{
	const fs::path& folderPath = folder->getPath();
	if(!fs::is_directory(folderPath)) { LOG(LogWarning) << "Error - folder not a directory!"; return; }
	const std::string folderStr = folderPath.generic_string();
	if(fs::is_symlink(folderPath))
	{
		if(folderStr.find(fs::canonical(folderPath).generic_string()) == 0) { LOG(LogWarning) << "Skipping recursive symlink"; return; }
	}
	fs::path filePath;
	std::string extension;
	bool isGame;
	for(fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		filePath = (*dir).path();
		if(filePath.stem().empty()) continue;
		extension = filePath.extension().string();
		isGame = false;
		if(std::find(mEnvData->mSearchExtensions.begin(), mEnvData->mSearchExtensions.end(), extension) != mEnvData->mSearchExtensions.end())
		{
			FileData* newGame = new FileData(GAME, filePath.generic_string(), this);
			folder->addChild(newGame);
			isGame = true;
		}
		if(!isGame && fs::is_directory(filePath))
		{
			FileData* newFolder = new FileData(FOLDER, filePath.generic_string(), this);
			populateFolder(newFolder);
			if(newFolder->getChildren().size() == 0) delete newFolder;
			else folder->addChild(newFolder);
		}
	}
}

std::vector<std::string> readList(const std::string& str, const char* delims = " \t\r\n,")
{
	std::vector<std::string> ret;
	size_t prevOff = str.find_first_not_of(delims, 0);
	size_t off = str.find_first_of(delims, prevOff);
	while(off != std::string::npos || prevOff != std::string::npos)
	{
		ret.push_back(str.substr(prevOff, off - prevOff));
		prevOff = str.find_first_not_of(delims, off);
		off = str.find_first_of(delims, prevOff);
	}
	return ret;
}

bool SystemData::loadConfig(Window* window)
{
	deleteSystems();
	std::string path = getConfigPath(false);
	LOG(LogInfo) << "Loading system config file " << path << "...";

	if(!fs::exists(path)) { LOG(LogError) << "es_systems.cfg file does not exist!"; writeExampleConfig(getConfigPath(true)); return false; }

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());
	if(!res) { LOG(LogError) << "Could not parse es_systems.cfg file!"; return false; }

	pugi::xml_node systemList = doc.child("systemList");
	if(!systemList) { LOG(LogError) << "es_systems.cfg is missing <systemList>!"; return false; }

	for(pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		std::string name, fullname, syspath, cmd, themeFolder;
		name = system.child("name").text().get();
		fullname = system.child("fullname").text().get();
		syspath = system.child("path").text().get();
		std::vector<std::string> extensions = readList(system.child("extension").text().get());
		cmd = system.child("command").text().get();
		const char* platformList = system.child("platform").text().get();
		std::vector<std::string> platformStrs = readList(platformList);
		std::vector<PlatformIds::PlatformId> platformIds;
		for(auto it = platformStrs.begin(); it != platformStrs.end(); it++)
		{
			const char* str = it->c_str();
			PlatformIds::PlatformId platformId = PlatformIds::getPlatformId(str);
			if(platformId == PlatformIds::PLATFORM_IGNORE) { platformIds.clear(); platformIds.push_back(platformId); break; }
			if(str != NULL && str[0] != '\0' && platformId == PlatformIds::PLATFORM_UNKNOWN)
				LOG(LogWarning) << "  Unknown platform for system \"" << name << "\"";
			else if(platformId != PlatformIds::PLATFORM_UNKNOWN)
				platformIds.push_back(platformId);
		}
		themeFolder = system.child("theme").text().as_string(name.c_str());
		if(name.empty() || syspath.empty() || extensions.empty() || cmd.empty())
		{ LOG(LogError) << "System \"" << name << "\" is missing name, path, extension, or command!"; continue; }

		boost::filesystem::path genericPath(syspath);
		syspath = genericPath.generic_string();

		SystemEnvironmentData* sysData = new SystemEnvironmentData{syspath, extensions, cmd, platformIds};
		SystemData* newSys = new SystemData(name, fullname, sysData, themeFolder);
		if(newSys->getRootFolder()->getChildren().size() == 0)
		{ LOG(LogWarning) << "System \"" << name << "\" has no games! Ignoring it."; delete newSys; }
		else { sSystemVector.push_back(newSys); }
	}
	return true;
}

void SystemData::writeExampleConfig(const std::string& path)
{
	std::ofstream file(path.c_str());
	file << "<systemList>\n  <system>\n    <name>nes</name>\n    <fullname>Nintendo Entertainment System</fullname>\n    <path>~/roms/nes</path>\n    <extension>.nes .NES</extension>\n    <command>retroarch -L ~/cores/libretro-fceumm.so %ROM%</command>\n    <platform>nes</platform>\n    <theme>nes</theme>\n  </system>\n</systemList>\n";
	file.close();
}

void SystemData::deleteSystems()
{
	for(unsigned int i = 0; i < sSystemVector.size(); i++)
		delete sSystemVector.at(i);
	sSystemVector.clear();
}

std::string SystemData::getConfigPath(bool forWrite)
{
	fs::path path = getHomePath() + "/.emulationstation/es_systems.cfg";
	if(forWrite || fs::exists(path)) return path.generic_string();
	return "/etc/emulationstation/es_systems.cfg";
}

std::string SystemData::getGamelistPath(bool forWrite) const
{
	fs::path filePath;
	filePath = mRootFolder->getPath() / "gamelist.xml";
	if(fs::exists(filePath)) return filePath.generic_string();
	filePath = getHomePath() + "/.emulationstation/gamelists/" + mName + "/gamelist.xml";
	if(forWrite) fs::create_directories(filePath.parent_path());
	if(forWrite || fs::exists(filePath)) return filePath.generic_string();
	return "/etc/emulationstation/gamelists/" + mName + "/gamelist.xml";
}

std::string SystemData::getThemePath() const
{
	fs::path localThemePath = mRootFolder->getPath() / "theme.xml";
	if(fs::exists(localThemePath)) return localThemePath.generic_string();
	return ThemeData::getThemeFromCurrentSet(mThemeFolder).generic_string();
}

bool SystemData::hasGamelist() const { return (fs::exists(getGamelistPath(false))); }
unsigned int SystemData::getGameCount() const { return mRootFolder->getFilesRecursive(GAME).size(); }

void SystemData::loadTheme()
{
	mTheme = std::make_shared<ThemeData>();
	std::string path = getThemePath();
	if(!fs::exists(path)) return;
	try { mTheme->loadFile(path); } catch(ThemeException& e) { LOG(LogError) << e.what(); mTheme = std::make_shared<ThemeData>(); }
}

void SystemData::setIsGameSystemStatus()
{
	if(mIsCollectionSystem) { mIsGameSystem = false; return; }
	if(mEnvData->mStartPath.empty()) { mIsGameSystem = false; return; }
	mIsGameSystem = true;
}

void SystemData::indexAllGameFilters(const FileData* folder)
{
	for(auto it = folder->getChildren().cbegin(); it != folder->getChildren().cend(); it++)
	{
		if((*it)->getType() == FOLDER) indexAllGameFilters(*it);
		else if(mFilterIndex != NULL) mFilterIndex->addToIndex(*it);
	}
}
