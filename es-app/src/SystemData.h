#pragma once

#include <vector>
#include <string>
#include <memory>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"

class FileFilterIndex;

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions,
		const std::string& command, const std::vector<PlatformIds::PlatformId>& platformIds, const std::string& themeFolder);
	SystemData(const std::string& name, const std::string& fullName, const std::string& themeFolder, bool CollectionSystem);
	~SystemData();

	inline FileData* getRootFolder() const { return mRootFolder; };
	inline const std::string& getName() const { return mName; }
	inline const std::string& getFullName() const { return mFullName; }
	inline const std::string& getStartPath() const { return mStartPath; }
	inline const std::vector<std::string>& getExtensions() const { return mSearchExtensions; }
	inline const std::string& getThemeFolder() const { return mThemeFolder; }

	inline const std::vector<PlatformIds::PlatformId>& getPlatformIds() const { return mPlatformIds; }
	inline bool hasPlatformId(PlatformIds::PlatformId id) { return std::find(mPlatformIds.begin(), mPlatformIds.end(), id) != mPlatformIds.end(); }

	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

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

	inline std::vector<SystemData*>::const_iterator getIterator() const { return std::find(sSystemVector.begin(), sSystemVector.end(), this); };
	inline std::vector<SystemData*>::const_reverse_iterator getRevIterator() const { return std::find(sSystemVector.rbegin(), sSystemVector.rend(), this); };

	inline bool isCollection() { return mIsCollectionSystem; }
	inline bool isGameSystem() { return mIsGameSystem; }
	inline FileFilterIndex* getIndex() { return mFilterIndex; }

	inline SystemData* getNext() const
	{
		auto it = getIterator();
		it++;
		if(it == sSystemVector.end()) it = sSystemVector.begin();
		return *it;
	}

	inline SystemData* getPrev() const
	{
		auto it = getRevIterator();
		it++;
		if(it == sSystemVector.rend()) it = sSystemVector.rbegin();
		return *it;
	}

	void loadTheme();

	FileFilterIndex* mFilterIndex;

private:
	std::string mName;
	std::string mFullName;
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
	std::vector<PlatformIds::PlatformId> mPlatformIds;
	std::string mThemeFolder;
	std::shared_ptr<ThemeData> mTheme;

	bool mIsCollectionSystem;
	bool mIsGameSystem;

	void populateFolder(FileData* folder);
	void setIsGameSystemStatus();
	void indexAllGameFilters(const FileData* folder);

	FileData* mRootFolder;
};
