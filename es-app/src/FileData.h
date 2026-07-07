#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include "MetaData.h"

class SystemData;

enum FileType
{
	GAME = 1,
	FOLDER = 2
};

enum FileChangeType
{
	FILE_ADDED,
	FILE_METADATA_CHANGED,
	FILE_REMOVED,
	FILE_SORTED
};

const char* fileTypeToString(FileType type);
FileType stringToFileType(const char* str);
std::string removeParenthesis(const std::string& str);

class FileData
{
public:
	FileData(FileType type, const boost::filesystem::path& path, SystemData* system);
	virtual ~FileData();

	inline const std::string& getName() const { return metadata.get("name"); }
	inline FileType getType() const { return mType; }
	inline const boost::filesystem::path& getPath() const { return mPath; }
	inline FileData* getParent() const { return mParent; }
	inline const std::vector<FileData*>& getChildren() const { return mChildren; }
	inline std::unordered_map<std::string, FileData*> getChildrenByFilename() const { return mChildrenByFilename; }
	inline SystemData* getSystem() const { return mSystem; }
	virtual const std::string& getThumbnailPath() const;

	std::string getCleanName() const;
	std::vector<FileData*> getFilesRecursive(unsigned int typeMask) const;
	void addChild(FileData* file);
	void removeChild(FileData* file);

	typedef bool ComparisonFunction(const FileData* a, const FileData* b);

	struct SortType
	{
		ComparisonFunction* comparisonFunction;
		bool ascending;
		std::string description;
		SortType(ComparisonFunction* sortFunction, bool sortAscending, const std::string & sortDescription)
			: comparisonFunction(sortFunction), ascending(sortAscending), description(sortDescription) {}
	};

	void sort(ComparisonFunction& comparator, bool ascending = true);
	void sort(const SortType& type);

	MetaDataList metadata;

protected:
	FileType mType;
	boost::filesystem::path mPath;
	SystemData* mSystem;
	FileData* mParent;
	std::vector<FileData*> mChildren;
	std::unordered_map<std::string, FileData*> mChildrenByFilename;
};

class CollectionFileData : public FileData
{
public:
	CollectionFileData(FileData* file, SystemData* system);
	~CollectionFileData();
	const std::string& getName();
	void refreshMetadata();
	FileData* getSourceFileData();
	std::string getKey();
private:
	FileData* mSourceFileData;
	std::string mCollectionFileName;
	bool mDirty;
};

FileData::SortType getSortTypeFromString(std::string desc);
