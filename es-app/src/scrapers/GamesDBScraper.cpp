#include "scrapers/GamesDBScraper.h"
#include "Log.h"
#include "Settings.h"
#include "Util.h"
#include <boost/assign.hpp>

using namespace PlatformIds;
const std::map<PlatformId, int> gamesdb_platformid_map = boost::assign::map_list_of
	(ARCADE, 1)
	(ATARI_2600, 26)
	(ATARI_5200, 40)
	(ATARI_7800, 41)
	(ATARI_JAGUAR, 27)
	(ATARI_LYNX, 28)
	(COLECOVISION, 48)
	(GAME_BOY, 9)
	(GAME_BOY_ADVANCE, 12)
	(GAME_BOY_COLOR, 10)
	(NEOGEO, 142)
	(NEOGEO_POCKET, 25)
	(NEOGEO_POCKET_COLOR, 82)
	(NINTENDO_64, 14)
	(NINTENDO_ENTERTAINMENT_SYSTEM, 3)
	(SEGA_32X, 19)
	(SEGA_CD, 20)
	(SEGA_GAME_GEAR, 21)
	(SEGA_GENESIS, 1)
	(SEGA_MASTER_SYSTEM, 2)
	(SEGA_MEGA_DRIVE, 1)
	(PLAYSTATION, 57)
	(PLAYSTATION_PORTABLE, 61)
	(SUPER_NINTENDO, 4)
	(TURBOGRAFX_16, 31)
	(WONDERSWAN, 45)
	(WONDERSWAN_COLOR, 46);

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, 
	std::vector<ScraperSearchResult>& results)
{
	std::string apiKey = Settings::getInstance()->getString("TheGamesDBApiKey");
	if(apiKey.empty())
	{
		LOG(LogWarning) << "TheGamesDB: No API key configured. Set TheGamesDBApiKey in settings.";
		return;
	}

	std::string name = params.nameOverride;
	if(name.empty())
		name = params.game->getCleanName();

	std::string url = "https://api.thegamesdb.net/v1/Games/ByGameName?";
	url += "name=" + HttpReq::urlEncode(name);
	url += "&apikey=" + HttpReq::urlEncode(apiKey);
	url += "&include=boxart";

	if(!params.system->getPlatformIds().empty())
	{
		PlatformId id = params.system->getPlatformIds()[0];
		auto it = gamesdb_platformid_map.find(id);
		if(it != gamesdb_platformid_map.end())
		{
			url += "&filter[platform_id]=" + std::to_string(it->second);
		}
	}

	LOG(LogInfo) << "TheGamesDB: " << url;
	requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, url)));
}

static std::string jsonGetString(const std::string& json, const std::string& key)
{
	size_t pos = json.find("\"" + key + "\"");
	if(pos == std::string::npos) return "";
	pos = json.find(":", pos);
	if(pos == std::string::npos) return "";
	pos = json.find("\"", pos + 1);
	if(pos == std::string::npos) return "";
	size_t end = json.find("\"", pos + 1);
	if(end == std::string::npos) return "";
	return json.substr(pos + 1, end - pos - 1);
}

void TheGamesDBRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	if(req->status() != HttpReq::REQ_SUCCESS)
	{
		LOG(LogError) << "TheGamesDB: HTTP error " << req->getErrorMsg();
		return;
	}

	const std::string content = req->getContent();
	size_t pos = 0;

	while((pos = content.find("\"game_title\"", pos)) != std::string::npos)
	{
		size_t blockStart = pos;
		size_t blockEnd = content.find("}", pos);
		if(blockEnd == std::string::npos) break;

		std::string block = content.substr(blockStart, blockEnd - blockStart + 1);

		ScraperSearchResult result;

		std::string title = jsonGetString(block, "game_title");
		if(title.empty()) { pos = blockEnd + 1; continue; }
		result.mdl.set("name", title);

		std::string overview = jsonGetString(block, "overview");
		if(!overview.empty()) result.mdl.set("desc", overview);

		std::string releaseDate = jsonGetString(block, "release_date");
		if(!releaseDate.empty()) result.mdl.set("releasedate", releaseDate);

		std::string rating = jsonGetString(block, "rating");
		if(!rating.empty()) result.mdl.set("rating", rating);

		size_t boxartPos = block.find("\"original\"");
		if(boxartPos != std::string::npos)
		{
			size_t urlStart = block.find("\"", boxartPos + 11);
			if(urlStart != std::string::npos)
			{
				size_t urlEnd = block.find("\"", urlStart + 1);
				if(urlEnd != std::string::npos)
					result.imageUrl = block.substr(urlStart + 1, urlEnd - urlStart - 1);
			}
		}

		results.push_back(result);
		pos = blockEnd + 1;
	}

	LOG(LogInfo) << "TheGamesDB: found " << results.size() << " results";
}
