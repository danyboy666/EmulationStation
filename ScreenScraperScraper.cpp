#include "scrapers/ScreenScraperScraper.h"
#include "Log.h"
#include "Settings.h"
#include "Util.h"
#include <boost/assign.hpp>

using namespace PlatformIds;

const std::map<PlatformId, int> screenscraper_platformid_map = boost::assign::map_list_of
	(ARCADE, 1)
	(ATARI_2600, 26)
	(ATARI_5200, 40)
	(ATARI_7800, 41)
	(COLECOVISION, 48)
	(GAME_BOY, 9)
	(GAME_BOY_ADVANCE, 12)
	(GAME_BOY_COLOR, 10)
	(NEOGEO, 142)
	(NEOGEO_POCKET, 25)
	(NEOGEO_POCKET_COLOR, 82)
	(NINTENDO_64, 14)
	(NINTENDO_ENTERTAINMENT_SYSTEM, 3)
	(NINTENDO_GAMECUBE, 13)
	(SEGA_32X, 19)
	(SEGA_CD, 20)
	(SEGA_DREAMCAST, 23)
	(SEGA_GAME_GEAR, 21)
	(SEGA_GENESIS, 1)
	(SEGA_MASTER_SYSTEM, 2)
	(SEGA_MEGA_DRIVE, 1)
	(SEGA_SATURN, 22)
	(SEGA_SG1000, 109)
	(PLAYSTATION, 57)
	(PLAYSTATION_PORTABLE, 61)
	(SUPER_NINTENDO, 4)
	(TURBOGRAFX_16, 31)
	(WONDERSWAN, 45)
	(WONDERSWAN_COLOR, 46)
	(ATARI_LYNX, 28)
	(ATARI_JAGUAR, 27)
	(ATARI_2600, 26)
	(ATARI_5200, 40)
	(ATARI_7800, 41)
	(COLECOVISION, 48)
	(NINTENDO_ENTERTAINMENT_SYSTEM, 3)
	(GAME_BOY, 9)
	(GAME_BOY_ADVANCE, 12)
	(GAME_BOY_COLOR, 10)
	(VIRTUAL_BOY, 11);

void screenscraper_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, 
	std::vector<ScraperSearchResult>& results)
{
	std::string name = params.nameOverride;
	if(name.empty())
		name = params.game->getCleanName();

	std::string system = "";
	if(!params.system->getPlatformIds().empty())
	{
		PlatformId id = params.system->getPlatformIds()[0];
		auto it = screenscraper_platformid_map.find(id);
		if(it != screenscraper_platformid_map.end())
		{
			char sysId[16];
			snprintf(sysId, sizeof(sysId), "%d", it->second);
			system = sysId;
		}
	}

	// ScreenScraper API v2
	// Use softname for anonymous access (limited to 2 requests/minute)
	std::string url = "https://api.screenscraper.fr/api2/jeuRecherche.php?";
	url += "softname=PS4RetroBox";
	url += "&rechte=0";  // 0 = non-registered user
	url += "&output=XML";
	url += "&romnom=" + HttpReq::urlEncode(name);
	if(!system.empty())
		url += "&systemeid=" + system;

	LOG(LogInfo) << "ScreenScraper: " << url;
	requests.push_back(std::unique_ptr<ScraperRequest>(new ScreenScraperRequest(results, url)));
}

void ScreenScraperRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	if(req->status() != 200)
	{
		LOG(LogError) << "ScreenScraper: HTTP error " << req->status();
		return;
	}

	const std::string& content = req->content();

	// Simple XML parsing for ScreenScraper response
	// The response contains <jeu> elements with game data
	size_t pos = 0;
	while((pos = content.find("<jeu>", pos)) != std::string::npos)
	{
		size_t end = content.find("</jeu>", pos);
		if(end == std::string::npos) break;

		std::string gameXml = content.substr(pos, end - pos + 6);

		ScraperSearchResult result;

		// Extract name
		size_t namePos = gameXml.find("<nom>");
		if(namePos != std::string::npos)
		{
			size_t nameEnd = gameXml.find("</nom>", namePos);
			if(nameEnd != std::string::npos)
			{
				result.name = gameXml.substr(namePos + 5, nameEnd - namePos - 5);
			}
		}

		// Extract description
		size_t descPos = gameXml.find("<synopsis>");
		if(descPos != std::string::npos)
		{
			size_t descEnd = gameXml.find("</synopsis>", descPos);
			if(descEnd != std::string::npos)
			{
				result.description = gameXml.substr(descPos + 10, descEnd - descPos - 10);
			}
		}

		// Extract year
		size_t yearPos = gameXml.find("<annee>");
		if(yearPos != std::string::npos)
		{
			size_t yearEnd = gameXml.find("</annee>", yearPos);
			if(yearEnd != std::string::npos)
				result.releaseDate = gameXml.substr(yearPos + 7, yearEnd - yearPos - 7);
		}

		// Extract developer
		size_t devPos = gameXml.find("<developpeur>");
		if(devPos != std::string::npos)
		{
			size_t devEnd = gameXml.find("</developpeur>", devPos);
			if(devEnd != std::string::npos)
				result.developer = gameXml.substr(devPos + 13, devEnd - devPos - 13);
		}

		// Extract publisher
		size_t pubPos = gameXml.find("<editeur>");
		if(pubPos != std::string::npos)
		{
			size_t pubEnd = gameXml.find("</editeur>", pubPos);
			if(pubEnd != std::string::npos)
				result.publisher = gameXml.substr(pubPos + 9, pubEnd - pubPos - 9);
		}

		// Extract genre
		size_t genrePos = gameXml.find("<genre>");
		if(genrePos != std::string::npos)
		{
			size_t genreEnd = gameXml.find("</genre>", genrePos);
			if(genreEnd != std::string::npos)
				result.genre = gameXml.substr(genrePos + 7, genreEnd - genrePos - 7);
		}

		// Extract players
		size_t playersPos = gameXml.find("<nbplayers>");
		if(playersPos != std::string::npos)
		{
			size_t playersEnd = gameXml.find("</nbplayers>", playersPos);
			if(playersEnd != std::string::npos)
				result.players = gameXml.substr(playersPos + 11, playersEnd - playersPos - 11);
		}

		// Extract rating
		size_t ratingPos = gameXml.find("<note>");
		if(ratingPos != std::string::npos)
		{
			size_t ratingEnd = gameXml.find("</note>", ratingPos);
			if(ratingEnd != std::string::npos)
				result.rating = gameXml.substr(ratingPos + 6, ratingEnd - ratingPos - 6);
		}

		// Extract boxart URLs
		size_t boxartPos = gameXml.find("<media_boxart>");
		if(boxartPos != std::string::npos)
		{
			size_t boxartEnd = gameXml.find("</media_boxart>", boxartPos);
			if(boxartEnd != std::string::npos)
			{
				std::string boxart = gameXml.substr(boxartPos + 14, boxartEnd - boxartPos - 14);
				if(!boxart.empty())
					result.imageUrl = "https://images.screenscraper.fr/medias/" + boxart;
			}
		}

		result.status = ScraperSearchResult::READY;
		results.push_back(result);
		pos = end + 6;
	}

	LOG(LogInfo) << "ScreenScraper: found " << results.size() << " results";
}
