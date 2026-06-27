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
	requests.push_back(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, url)));
}

void TheGamesDBRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	if(req->status() != 200)
	{
		LOG(LogError) << "TheGamesDB: HTTP error " << req->status();
		return;
	}

	const std::string& content = req->content();

	// Parse JSON response
	// TheGamesDB v1 returns JSON with game data
	size_t pos = 0;
	while((pos = content.find("\"game_title\"", pos)) != std::string::npos)
	{
		ScraperSearchResult result;

		// Extract game_title
		size_t end = content.find(",", pos);
		if(end != std::string::npos)
		{
			result.name = content.substr(pos + 14, end - pos - 15);
			// Remove quotes
			if(result.name.front() == '"') result.name = result.name.substr(1);
			if(result.name.back() == '"') result.name.pop_back();
		}

		// Extract overview/description
		size_t descPos = content.find("\"overview\"", pos);
		if(descPos != std::string::npos)
		{
			size_t descEnd = content.find("\",", descPos);
			if(descEnd != std::string::npos)
			{
				result.description = content.substr(descPos + 12, descEnd - descPos - 13);
				if(result.description.front() == '"') result.description = result.description.substr(1);
			}
		}

		// Extract release date
		size_t datePos = content.find("\"release_date\"", pos);
		if(datePos != std::string::npos)
		{
			size_t dateEnd = content.find("\",", datePos);
			if(dateEnd != std::string::npos)
				result.releaseDate = content.substr(datePos + 15, dateEnd - datePos - 16);
		}

		// Extract developers
		size_t devPos = content.find("\"developers\"", pos);
		if(devPos != std::string::npos)
		{
			size_t devEnd = content.find("]", devPos);
			if(devEnd != std::string::npos)
				result.developer = content.substr(devPos + 13, devEnd - devPos - 14);
		}

		// Extract publishers
		size_t pubPos = content.find("\"publishers\"", pos);
		if(pubPos != std::string::npos)
		{
			size_t pubEnd = content.find("]", pubPos);
			if(pubEnd != std::string::npos)
				result.publisher = content.substr(pubPos + 14, pubEnd - pubPos - 15);
		}

		// Extract genres
		size_t genrePos = content.find("\"genres\"", pos);
		if(genrePos != std::string::npos)
		{
			size_t genreEnd = content.find("]", genrePos);
			if(genreEnd != std::string::npos)
				result.genre = content.substr(genrePos + 9, genreEnd - genrePos - 10);
		}

		// Extract rating
		size_t ratingPos = content.find("\"rating\"", pos);
		if(ratingPos != std::string::npos)
		{
			size_t ratingEnd = content.find(",", ratingPos);
			if(ratingEnd != std::string::npos)
				result.rating = content.substr(ratingPos + 9, ratingEnd - ratingPos - 10);
		}

		// Extract boxart URL
		size_t boxartPos = content.find("\"original\"", pos);
		if(boxartPos != std::string::npos)
		{
			size_t boxartEnd = content.find("\"", boxartPos + 12);
			if(boxartEnd != std::string::npos)
				result.imageUrl = content.substr(boxartPos + 12, boxartEnd - boxartPos - 12);
		}

		result.status = ScraperSearchResult::READY;
		results.push_back(result);
		
		pos = (end != std::string::npos) ? end + 1 : content.size();
	}

	LOG(LogInfo) << "TheGamesDB: found " << results.size() << " results";
}
