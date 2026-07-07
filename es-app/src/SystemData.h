#pragma once

#include <vector>
#include <string>
#include <memory>
#include <random>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"

class FileFilterIndex;

struct SystemEnvironmentData
{
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
	std::vector<PlatformIds::PlatformId> mPlatformIds;
};

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, SystemEnvironmentData* envData, const std::string& themeFolder, bool CollectionSystem = false);
	~SystemData();

	inline FileData* getRootFolder() const { return mRootFolder; };
	inline const std::string& getName() const { return mName; }
	inline const std::string& getFullName() const { return mFullName; }
	inline const std::string& getStartPath() const { return mEnvData->mStartPath; }
	inline const std::vector<std::string>& getExtensions() const { return mEnvData->mSearchExtensions; }
	inline const std::string& getThemeFolder() const { return mThemeFolder; }
	inline SystemEnvironmentData* getSystemEnvData() const { return mEnvData; }
	inline const std::vector<PlatformIds::PlatformId>& getPlatformIds() const { return mEnvData->mPlatformIds; }
	inline bool hasPlatformId(PlatformIds::PlatformId id) { if (!mEnvData) return false; return std::find(mEnvData->mPlatformIds.cbegin(), mEnvData->mPlatformIds.cend(), id) != mEnvData->mPlatformIds.cend(); }
	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }
	inline bool isCollection() { return mIsCollectionSystem; }
	inline bool isGameSystem() { return mIsGameSystem; }
	inline bool isVisible() { return mIsGameSystem || mIsCollectionSystem; }

	std::string getGamelistPath(bool forWrite) const;
	bool hasGamelist() const;
	std::string getThemePath() const;
	unsigned int getGameCount() const;
	void launchGame(Window* window, FileData* game);

	static void deleteSystems();
	static bool loadConfig(Window* window);
	static void writeExampleConfig(const std::string& path);
	static std::string getConfigPath(bool forWrite);

	static std::vector<SystemData*> sSystemVector;

	inline std::vector<SystemData*>::const_iterator getIterator() const { return std::find(sSystemVector.cbegin(), sSystemVector.cend(), this); };
	inline std::vector<SystemData*>::const_reverse_iterator getRevIterator() const { return std::find(sSystemVector.crbegin(), sSystemVector.crend(), this); };
	inline SystemData* getNext() const { auto it = getIterator(); it++; if(it == sSystemVector.cend()) it = sSystemVector.cbegin(); return *it; }
	inline SystemData* getPrev() const { auto it = getRevIterator(); it++; if(it == sSystemVector.crend()) it = sSystemVector.crbegin(); return *it; }

	void loadTheme();
	FileFilterIndex* getIndex() { return mFilterIndex; }

	bool mIsCollectionSystem;
	bool mIsGameSystem;

private:
	std::string mName;
	std::string mFullName;
	std::string mThemeFolder;
	std::shared_ptr<ThemeData> mTheme;
	SystemEnvironmentData* mEnvData;
	FileFilterIndex* mFilterIndex;
	FileData* mRootFolder;

	void populateFolder(FileData* folder);
	void indexAllGameFilters(const FileData* folder);
	void setIsGameSystemStatus();
};
