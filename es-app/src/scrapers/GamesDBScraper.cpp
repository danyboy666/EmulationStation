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

static std::string jsonGetInt(const std::string& json, const std::string& key)
{
	size_t pos = json.find("\"" + key + "\"");
	if(pos == std::string::npos) return "";
	pos = json.find(":", pos);
	if(pos == std::string::npos) return "";
	size_t start = pos + 1;
	while(start < json.size() && json[start] == ' ') start++;
	size_t end = start;
	while(end < json.size() && (std::isdigit(json[end]) || json[end] == '.' || json[end] == '-')) end++;
	if(end == start) return "";
	return json.substr(start, end - start);
}

static std::string jsonFindNested(const std::string& json, const std::string& parentKey, const std::string& childKey)
{
	size_t pos = json.find("\"" + parentKey + "\"");
	if(pos == std::string::npos) return "";
	return jsonGetString(json.substr(pos), childKey);
}

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	std::string apiKey = Settings::getInstance()->getString("TheGamesDBApiKey");
	if(apiKey.empty())
	{
		LOG(LogWarning) << "TheGamesDB: No API key configured. Set TheGamesDBApiKey in es_settings.xml.";
		return;
	}

	std::string name = params.nameOverride;
	if(name.empty())
		name = params.game->getCleanName();

	std::string url = "https://api.thegamesdb.net/v1/Games/ByGameName?";
	url += "apikey=" + HttpReq::urlEncode(apiKey);
	url += "&name=" + HttpReq::urlEncode(name);
	url += "&fields=overview,rating,players,publishers,genres,developers";
	url += "&include=boxart";

	if(!params.system->getPlatformIds().empty())
	{
		PlatformId id = params.system->getPlatformIds()[0];
		auto it = gamesdb_platformid_map.find(id);
		if(it != gamesdb_platformid_map.end())
		{
			url += "&filter[platform]=" + std::to_string(it->second);
		}
	}

	LOG(LogInfo) << "TheGamesDB: " << url;
	requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBRequest(results, url)));
}

void TheGamesDBRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	if(req->status() != HttpReq::REQ_SUCCESS)
	{
		LOG(LogError) << "TheGamesDB: HTTP error " << req->getErrorMsg();
		return;
	}

	const std::string content = req->getContent();

	std::string baseUrl;
	size_t baseUrlPos = content.find("\"original\"");
	if(baseUrlPos != std::string::npos)
	{
		size_t s = content.find("\"", baseUrlPos + 11);
		if(s != std::string::npos)
		{
			size_t e = content.find("\"", s + 1);
			if(e != std::string::npos)
				baseUrl = content.substr(s + 1, e - s - 1);
		}
	}

	size_t pos = 0;
	while((pos = content.find("\"game_title\"", pos)) != std::string::npos)
	{
		size_t gameStart = pos;
		size_t gameEnd = content.find("\"game_title\"", pos + 12);
		if(gameEnd == std::string::npos)
			gameEnd = content.find("\"pages\"", pos);
		if(gameEnd == std::string::npos)
			gameEnd = content.size();

		std::string gameBlock = content.substr(gameStart, gameEnd - gameStart);

		ScraperSearchResult result;

		std::string title = jsonGetString(gameBlock, "game_title");
		if(title.empty()) { pos = gameEnd; continue; }
		result.mdl.set("name", title);

		std::string overview = jsonGetString(gameBlock, "overview");
		if(!overview.empty()) result.mdl.set("desc", overview);

		std::string releaseDate = jsonGetString(gameBlock, "release_date");
		if(!releaseDate.empty()) result.mdl.set("releasedate", releaseDate);

		std::string rating = jsonGetString(gameBlock, "rating");
		if(!rating.empty())
		{
			float ratingVal = 0.0f;
			if(rating.find("E") != std::string::npos || rating.find("e") != std::string::npos)
				ratingVal = 0.5f;
			else if(rating.find("M") != std::string::npos || rating.find("m") != std::string::npos)
				ratingVal = 0.75f;
			else if(rating.find("T") != std::string::npos || rating.find("t") != std::string::npos)
				ratingVal = 0.6f;
			else
			{
				try { ratingVal = std::stof(rating); } catch(...) {}
			}
			char buf[16];
			snprintf(buf, sizeof(buf), "%.6f", ratingVal);
			result.mdl.set("rating", buf);
		}

		std::string players = jsonGetInt(gameBlock, "players");
		if(!players.empty()) result.mdl.set("players", players);

		std::string gameId = jsonGetInt(gameBlock, "id");

		if(!gameId.empty() && !baseUrl.empty())
		{
			size_t boxartPos = content.find("\"" + gameId + "\"", gameEnd);
			if(boxartPos != std::string::npos)
			{
				size_t filenamePos = content.find("\"filename\"", boxartPos);
				if(filenamePos != std::string::npos)
				{
					size_t s = content.find("\"", filenamePos + 10);
					if(s != std::string::npos)
					{
						size_t e = content.find("\"", s + 1);
						if(e != std::string::npos)
						{
							std::string filename = content.substr(s + 1, e - s - 1);
							std::string side = jsonGetString(content.substr(boxartPos), "side");
							if(side == "front" || side.empty())
								result.imageUrl = baseUrl + filename;
						}
					}
				}
			}
		}

		results.push_back(result);
		pos = gameEnd;
	}

	LOG(LogInfo) << "TheGamesDB: found " << results.size() << " results";
}
