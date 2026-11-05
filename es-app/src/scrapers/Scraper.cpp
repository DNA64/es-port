#include "scrapers/Scraper.h"

#include "FileData.h"
#include "GamesDBJSONScraper.h"
#include "ScreenScraper.h"
#include "Log.h"
#include "Settings.h"
#include "SystemData.h"
#include "platform.h"
#include "utils/FileSystemUtil.h"
#include <FreeImage.h>
#include <algorithm>
#include <fstream>

const std::map<std::string, generate_scraper_requests_func> scraper_request_funcs {
	{ "TheGamesDB", &thegamesdb_generate_json_scraper_requests },
	{ "ScreenScraper", &screenscraper_generate_scraper_requests }
};

std::unique_ptr<ScraperSearchHandle> startScraperSearch(const ScraperSearchParams& params)
{
	const std::string& name = Settings::getInstance()->getString("Scraper");
	std::unique_ptr<ScraperSearchHandle> handle(new ScraperSearchHandle());

	// Check if the Scraper in the settings still exists as a registered scraping source.
	if (scraper_request_funcs.find(name) == scraper_request_funcs.end())
	{
		LOG(LogWarning) << "Configured scraper (" << name << ") unavailable, scraping aborted.";
	}
	else
	{
		scraper_request_funcs.at(name)(params, handle->mRequestQueue, handle->mResults);
	}

	return handle;
}

std::vector<std::string> getScraperList()
{
	std::vector<std::string> list;
	for(auto it = scraper_request_funcs.cbegin(); it != scraper_request_funcs.cend(); it++)
	{
		list.push_back(it->first);
	}

	return list;
}

bool isValidConfiguredScraper()
{
	const std::string& name = Settings::getInstance()->getString("Scraper");
	return scraper_request_funcs.find(name) != scraper_request_funcs.end();
}

// ScraperSearchHandle
ScraperSearchHandle::ScraperSearchHandle()
{
	setStatus(ASYNC_IN_PROGRESS);
}

void ScraperSearchHandle::update()
{
	if(mStatus == ASYNC_DONE)
		return;

	if(!mRequestQueue.empty())
	{
		// a request can add more requests to the queue while running,
		// so be careful with references into the queue
		auto& req = *(mRequestQueue.front());
		AsyncHandleStatus status = req.status();

		if(status == ASYNC_ERROR)
		{
			// propegate error
			setError(req.getStatusString());

			// empty our queue
			while(!mRequestQueue.empty())
				mRequestQueue.pop();

			return;
		}

		// finished this one, see if we have any more
		if(status == ASYNC_DONE)
		{
			mRequestQueue.pop();
		}

		// status == ASYNC_IN_PROGRESS
	}

	// we finished without any errors!
	if(mRequestQueue.empty())
	{
		setStatus(ASYNC_DONE);
		return;
	}
}



// ScraperRequest
ScraperRequest::ScraperRequest(std::vector<ScraperSearchResult>& resultsWrite) : mResults(resultsWrite)
{
}


// ScraperHttpRequest
namespace
{
	std::string maskedUrl(std::string url)
	{
		for(const char* key : { "devid=", "devpassword=", "apikey=" })
		{
			const std::size_t start = url.find(key);
			if(start == std::string::npos)
				continue;
			const std::size_t valueStart = start + std::string(key).size();
			const std::size_t end = url.find('&', valueStart);
			url.replace(valueStart, end == std::string::npos ? std::string::npos : end - valueStart, "***");
		}
		return url;
	}
}

ScraperHttpRequest::ScraperHttpRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url)
	: ScraperRequest(resultsWrite)
{
	setStatus(ASYNC_IN_PROGRESS);
	LOG(LogInfo) << "scraper request: " << maskedUrl(url);
	mReq = std::unique_ptr<HttpReq>(new HttpReq(url));
}

void ScraperHttpRequest::update()
{
	HttpReq::Status status = mReq->status();
	if(status == HttpReq::REQ_SUCCESS)
	{
		setStatus(ASYNC_DONE); // if process() has an error, status will be changed to ASYNC_ERROR
		const std::string& content = mReq->getContent();
		std::string head = content.substr(0, 100);
		std::replace(head.begin(), head.end(), '\n', ' ');
		LOG(LogInfo) << "scraper response: " << content.size() << " bytes: " << head;
		process(mReq, mResults);
		return;
	}

	// not ready yet
	if(status == HttpReq::REQ_IN_PROGRESS)
		return;

	// everything else is some sort of error
	LOG(LogError) << "ScraperHttpRequest network error (status: " << status << ") - " << mReq->getErrorMsg();
	setError(mReq->getErrorMsg());
}


// metadata resolving stuff

std::unique_ptr<MDResolveHandle> resolveMetaDataAssets(const ScraperSearchResult& result, const ScraperSearchParams& search)
{
	return std::unique_ptr<MDResolveHandle>(new MDResolveHandle(result, search));
}

MDResolveHandle::MDResolveHandle(const ScraperSearchResult& result, const ScraperSearchParams& search) : mResult(result)
{
	if(!Settings::getInstance()->getBool("ScrapeImage"))
		mResult.imageUrl.clear();

	if(!result.imageUrl.empty() && Settings::getInstance()->getBool("ScrapeImage"))
	{

		std::string ext;

		// If we have a file extension returned by the scraper, then use it.
		// Otherwise, try to guess it by the name of the URL, which point to an image.
		if (!result.imageType.empty())
		{
			ext = result.imageType;
		}else{
			size_t dot = result.imageUrl.find_last_of('.');

			if (dot != std::string::npos)
				ext = result.imageUrl.substr(dot, std::string::npos);
		}

		std::string imgPath = getSaveAsPath(search, "image", ext);

		mFuncs.push_back(ResolvePair(downloadImageAsync(result.imageUrl, imgPath), [this, imgPath]
		{
			mResult.mdl.set("image", imgPath);
			mResult.imageUrl = "";
		}));
	}
}

void MDResolveHandle::update()
{
	if(mStatus == ASYNC_DONE || mStatus == ASYNC_ERROR)
		return;

	auto it = mFuncs.cbegin();
	while(it != mFuncs.cend())
	{
		if(it->first->status() == ASYNC_ERROR)
		{
			setError(it->first->getStatusString());
			return;
		}else if(it->first->status() == ASYNC_DONE)
		{
			it->second();
			it = mFuncs.erase(it);
			continue;
		}
		it++;
	}

	if(mFuncs.empty())
		setStatus(ASYNC_DONE);
}

std::unique_ptr<ImageDownloadHandle> downloadImageAsync(const std::string& url, const std::string& saveAs)
{
	return std::unique_ptr<ImageDownloadHandle>(new ImageDownloadHandle(url, saveAs,
		Settings::getInstance()->getInt("ScraperResizeWidth"), Settings::getInstance()->getInt("ScraperResizeHeight")));
}

ImageDownloadHandle::ImageDownloadHandle(const std::string& url, const std::string& path, int maxWidth, int maxHeight) :
	mSavePath(path), mMaxWidth(maxWidth), mMaxHeight(maxHeight), mReq(new HttpReq(url))
{
}

void ImageDownloadHandle::update()
{
	if(mReq->status() == HttpReq::REQ_IN_PROGRESS)
		return;

	if(mReq->status() != HttpReq::REQ_SUCCESS)
	{
		std::stringstream ss;
		ss << "Network error: " << mReq->getErrorMsg();
		setError(ss.str());
		return;
	}

	// download is done, save it to disk
	std::ofstream stream(mSavePath, std::ios_base::out | std::ios_base::binary);
	if(stream.bad())
	{
		setError("Failed to open image path to write. Permission error? Disk full?");
		return;
	}

	const std::string& content = mReq->getContent();
	stream.write(content.data(), content.length());
	stream.close();
	if(stream.bad())
	{
		setError("Failed to save image. Disk full?");
		return;
	}

	// resize it
	if(!resizeImage(mSavePath, mMaxWidth, mMaxHeight))
	{
		setError("Error saving resized image. Out of memory? Disk full?");
		return;
	}

	setStatus(ASYNC_DONE);
}

//you can pass 0 for width or height to keep aspect ratio
bool resizeImage(const std::string& path, int maxWidth, int maxHeight)
{
	// nothing to do
	if(maxWidth == 0 && maxHeight == 0)
		return true;

	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* image = NULL;

	//detect the filetype
	format = FreeImage_GetFileType(path.c_str(), 0);
	if(format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(path.c_str());
	if(format == FIF_UNKNOWN)
	{
		LOG(LogError) << "Error - could not detect filetype for image \"" << path << "\"!";
		return false;
	}

	//make sure we can read this filetype first, then load it
	if(FreeImage_FIFSupportsReading(format))
	{
		image = FreeImage_Load(format, path.c_str());
	}else{
		LOG(LogError) << "Error - file format reading not supported for image \"" << path << "\"!";
		return false;
	}

	float width = (float)FreeImage_GetWidth(image);
	float height = (float)FreeImage_GetHeight(image);

	if(maxWidth == 0)
	{
		maxWidth = (int)((maxHeight / height) * width);
	}else if(maxHeight == 0)
	{
		maxHeight = (int)((maxWidth / width) * height);
	}

	FIBITMAP* imageRescaled = FreeImage_Rescale(image, maxWidth, maxHeight, FILTER_BILINEAR);
	FreeImage_Unload(image);

	if(imageRescaled == NULL)
	{
		LOG(LogError) << "Could not resize image! (not enough memory? invalid bitdepth?)";
		return false;
	}

	bool saved = (FreeImage_Save(format, imageRescaled, path.c_str()) != 0);
	FreeImage_Unload(imageRescaled);

	if(!saved)
		LOG(LogError) << "Failed to save resized image!";

	return saved;
}

std::string getSaveAsPath(const ScraperSearchParams& params, const std::string& suffix, const std::string& extension)
{
	const std::string subdirectory = params.system->getName();
	const std::string name = Utils::FileSystem::getStem(params.game->getPath()) + "-" + suffix;

	std::string path = Utils::FileSystem::isDirectory("/media/data")
		? "/media/data/emulationstation/downloaded_images/"
		: Utils::FileSystem::getHomePath() + "/.emulationstation/downloaded_images/";

	if(!Utils::FileSystem::exists(path))
		Utils::FileSystem::createDirectory(path);

	path += subdirectory + "/";

	if(!Utils::FileSystem::exists(path))
		Utils::FileSystem::createDirectory(path);


	path += name + extension;
	return path;
}

bool isScrapedKeyEnabled(const std::string& key)
{
	static const std::map<std::string, const char*> settingByKey {
		{ "name", "ScrapeName" },
		{ "desc", "ScrapeDescription" },
		{ "image", "ScrapeImage" },
		{ "rating", "ScrapeRatings" },
		{ "releasedate", "ScrapeDetails" },
		{ "developer", "ScrapeDetails" },
		{ "publisher", "ScrapeDetails" },
		{ "genre", "ScrapeDetails" },
		{ "players", "ScrapeDetails" }
	};

	const auto it = settingByKey.find(key);
	return it != settingByKey.cend() && Settings::getInstance()->getBool(it->second);
}

void applyScrapeResult(MetaDataList& target, const MetaDataList& result)
{
	for(const auto& decl : result.getMDD())
	{
		const bool keep = !Settings::getInstance()->getBool("ScrapeOverwrite") && decl.key != "name" && target.get(decl.key) != decl.defaultValue;
		if(isScrapedKeyEnabled(decl.key) && !keep && result.get(decl.key) != decl.defaultValue)
			target.set(decl.key, result.get(decl.key));
	}
}

namespace
{

std::string shellQuote(const std::string& value)
{
	std::string quoted = "'";
	for(const char c : value)
	{
		if(c == '\'')
			quoted += "'\''";
		else
			quoted += c;
	}
	return quoted + "'";
}

}

void saveScrapeToDesktop(const FileData* game, const MetaDataList& result)
{
	const std::string path = game->getPath();
	if(Utils::FileSystem::getExtension(path) != ".desktop")
		return;

	const struct { const char* key; const char* env; } fields[] = {
		{ "name", "ES_NAME" },
		{ "desc", "ES_DESC" },
		{ "releasedate", "ES_RELEASED" },
		{ "players", "ES_PLAYERS" },
		{ "publisher", "ES_PUBLISHER" },
		{ "genre", "ES_GENRE" }
	};

	std::string command;
	for(const auto& field : fields)
	{
		std::string value = result.get(field.key);
		if(!isScrapedKeyEnabled(field.key) || value.empty() || value == "unknown" || value == "not-a-date-time")
			continue;

		if(std::string(field.key) == "releasedate")
		{
			if(value.size() < 8)
				continue;
			value = value.substr(0, 4) + "-" + value.substr(4, 2) + "-" + value.substr(6, 2);
		}

		if(std::string(field.key) == "players")
		{
			const size_t last = value.find_last_of("0123456789");
			if(last == std::string::npos)
				continue;
			const size_t before = value.find_last_not_of("0123456789", last);
			const size_t first = before == std::string::npos ? 0 : before + 1;
			value = value.substr(first, last - first + 1);
		}

		command += std::string(field.env) + "=" + shellQuote(value) + " ";
	}

	if(!command.empty() && Settings::getInstance()->getBool("ScrapeOverwrite"))
		command += "ES_OVERWRITE=1 ";

	if(!command.empty())
		runSystemCommand(command + "sh /var/lib/hakchi/emulationstation/es-save-desktop.sh " + shellQuote(Utils::FileSystem::getStem(path)));
}

namespace
{

bool hasRomExtension(const std::string& exec, const std::string& extension)
{
	return exec.find(extension + ".") != std::string::npos || exec.find(extension + " ") != std::string::npos ||
		(exec.size() >= extension.size() && exec.compare(exec.size() - extension.size(), extension.size(), extension) == 0);
}

std::vector<PlatformIds::PlatformId> platformsFromExec(const std::string& exec)
{
	using namespace PlatformIds;
	if(exec.find("kachikachi") != std::string::npos || hasRomExtension(exec, ".nes"))
		return { NINTENDO_ENTERTAINMENT_SYSTEM };
	if(exec.find("shvc") != std::string::npos || exec.find("/bin/snes") != std::string::npos ||
		hasRomExtension(exec, ".sfc") || hasRomExtension(exec, ".smc"))
		return { SUPER_NINTENDO };
	if(hasRomExtension(exec, ".32x") || exec.find("/bin/32x") != std::string::npos)
		return { SEGA_32X };
	if(hasRomExtension(exec, ".sms"))
		return { SEGA_MASTER_SYSTEM };
	if(hasRomExtension(exec, ".gba"))
		return { GAME_BOY_ADVANCE };
	if(hasRomExtension(exec, ".gbc"))
		return { GAME_BOY_COLOR };
	if(hasRomExtension(exec, ".gb"))
		return { GAME_BOY };
	if(hasRomExtension(exec, ".pce"))
		return { TURBOGRAFX_16 };
	if(exec.find("/bin/md") != std::string::npos || exec.find("genesis") != std::string::npos ||
		hasRomExtension(exec, ".md") || hasRomExtension(exec, ".gen") || hasRomExtension(exec, ".bin"))
		return { SEGA_GENESIS, SEGA_MEGA_DRIVE };
	return {};
}

}

std::vector<PlatformIds::PlatformId> getScrapePlatforms(const ScraperSearchParams& params)
{
	std::ifstream desktop(params.game->getPath());
	std::string line;
	while(std::getline(desktop, line))
	{
		if(line.compare(0, 5, "Exec=") == 0)
		{
			const std::vector<PlatformIds::PlatformId> detected = platformsFromExec(line.substr(5));
			if(!detected.empty())
				return detected;
			break;
		}
	}

	return params.system->getPlatformIds();
}
