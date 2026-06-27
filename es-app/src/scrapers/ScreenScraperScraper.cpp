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
	(PLAYSTATION, 57)
	(PLAYSTATION_PORTABLE, 61)
	(SUPER_NINTENDO, 4)
	(TURBOGRAFX_16, 31)
	(WONDERSWAN, 45)
	(WONDERSWAN_COLOR, 46)
	(ATARI_LYNX, 28)
	(ATARI_JAGUAR, 27);

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

	std::string url = "https://api.screenscraper.fr/api2/jeuRecherche.php?";
	url += "softname=PS4RetroBox";
	url += "&output=XML";
	url += "&romnom=" + HttpReq::urlEncode(name);
	if(!system.empty())
		url += "&systemeid=" + system;

	std::string ssUser = Settings::getInstance()->getString("ScreenScraperUser");
	std::string ssPass = Settings::getInstance()->getString("ScreenScraperPass");
	if(!ssUser.empty() && !ssPass.empty())
	{
		url += "&devid=PS4RetroBox";
		url += "&user=" + HttpReq::urlEncode(ssUser);
		url += "&password=" + HttpReq::urlEncode(ssPass);
		LOG(LogInfo) << "ScreenScraper: using registered user " << ssUser;
	}
	else
	{
		url += "&rechte=0";
		LOG(LogInfo) << "ScreenScraper: using anonymous access (limited)";
	}

	LOG(LogInfo) << "ScreenScraper: " << url;
	requests.push(std::unique_ptr<ScraperRequest>(new ScreenScraperRequest(results, url)));
}

static std::string xmlGetTag(const std::string& xml, const std::string& tag)
{
	size_t pos = xml.find("<" + tag + ">");
	if(pos == std::string::npos) return "";
	size_t end = xml.find("</" + tag + ">", pos);
	if(end == std::string::npos) return "";
	return xml.substr(pos + tag.size() + 2, end - pos - tag.size() - 2);
}

void ScreenScraperRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	if(req->status() != HttpReq::REQ_SUCCESS)
	{
		LOG(LogError) << "ScreenScraper: HTTP error " << req->getErrorMsg();
		return;
	}

	const std::string content = req->getContent();
	size_t pos = 0;

	while((pos = content.find("<jeu>", pos)) != std::string::npos)
	{
		size_t end = content.find("</jeu>", pos);
		if(end == std::string::npos) break;

		std::string gameXml = content.substr(pos, end - pos + 6);

		ScraperSearchResult result;

		std::string name = xmlGetTag(gameXml, "nom");
		if(!name.empty()) result.mdl.set("name", name);

		std::string desc = xmlGetTag(gameXml, "synopsis");
		if(!desc.empty()) result.mdl.set("desc", desc);

		std::string year = xmlGetTag(gameXml, "annee");
		if(!year.empty()) result.mdl.set("releasedate", year);

		std::string dev = xmlGetTag(gameXml, "developpeur");
		if(!dev.empty()) result.mdl.set("developer", dev);

		std::string pub = xmlGetTag(gameXml, "editeur");
		if(!pub.empty()) result.mdl.set("publisher", pub);

		std::string genre = xmlGetTag(gameXml, "genre");
		if(!genre.empty()) result.mdl.set("genre", genre);

		std::string players = xmlGetTag(gameXml, "nbplayers");
		if(!players.empty()) result.mdl.set("players", players);

		std::string rating = xmlGetTag(gameXml, "note");
		if(!rating.empty()) result.mdl.set("rating", rating);

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

		results.push_back(result);
		pos = end + 6;
	}

	LOG(LogInfo) << "ScreenScraper: found " << results.size() << " results";
}
