//EmulationStation, a graphical front-end for ROM browsing. Created by Alec "Aloshi" Lofquist.
//http://www.aloshi.com

#include "guis/GuiDetectDevice.h"
#include "AudioManager.h"
#include "guis/GuiMsgBox.h"
#include "utils/FileSystemUtil.h"
#include "utils/ProfilingUtil.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "EmulationStation.h"
#include "Gamelist.h"
#include "InputManager.h"
#include "Log.h"
#include "MameNames.h"
#include "platform.h"
#include "PowerSaver.h"
#include "ScraperCmdLine.h"
#include "MenuColors.h"
#include "Settings.h"
#include "SystemData.h"
#include "SystemScreenSaver.h"
#include "components/VideoVlcComponent.h"
#include <SDL_events.h>
#include <SDL_main.h>
#include <SDL_timer.h>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <time.h>
#include <csignal>
#include <fcntl.h>
#include <ucontext.h>
#include <unistd.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include <FreeImage.h>

bool scrape_cmdline = false;

bool parseArgs(int argc, char* argv[])
{
	Utils::FileSystem::setExePath(argv[0]);

	// We need to process --home before any call to Settings::getInstance(), because settings are loaded from homepath
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--home") == 0)
		{
			if(i >= argc - 1)
			{
				std::cerr << "Invalid home path supplied.";
				return false;
			}

			Utils::FileSystem::setHomePath(argv[i + 1]);
			break;
		}
	}

	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--monitor") == 0)
		{
			if (i >= argc - 1)
			{
				std::cerr << "Invalid monitor supplied.";
				return false;
			}

			int monitor = atoi(argv[i + 1]);
			i++; // skip the argument value
			Settings::getInstance()->setInt("MonitorID", monitor);
		}else if(strcmp(argv[i], "--resolution") == 0)
		{
			if(i >= argc - 2)
			{
				std::cerr << "Invalid resolution supplied.";
				return false;
			}

			int width = atoi(argv[i + 1]);
			int height = atoi(argv[i + 2]);
			i += 2; // skip the argument value
			Settings::getInstance()->setInt("WindowWidth", width);
			Settings::getInstance()->setInt("WindowHeight", height);
		}else if(strcmp(argv[i], "--screensize") == 0)
		{
			if(i >= argc - 2)
			{
				std::cerr << "Invalid screensize supplied.";
				return false;
			}

			int width = atoi(argv[i + 1]);
			int height = atoi(argv[i + 2]);
			i += 2; // skip the argument value
			Settings::getInstance()->setInt("ScreenWidth", width);
			Settings::getInstance()->setInt("ScreenHeight", height);
		}else if(strcmp(argv[i], "--screenoffset") == 0)
		{
			if(i >= argc - 2)
			{
				std::cerr << "Invalid screenoffset supplied.";
				return false;
			}

			int x = atoi(argv[i + 1]);
			int y = atoi(argv[i + 2]);
			i += 2; // skip the argument value
			Settings::getInstance()->setInt("ScreenOffsetX", x);
			Settings::getInstance()->setInt("ScreenOffsetY", y);
		}else if (strcmp(argv[i], "--screenrotate") == 0)
		{
			if (i >= argc - 1)
			{
				std::cerr << "Invalid screenrotate supplied.";
				return false;
			}

			int rotate = atoi(argv[i + 1]);
			++i; // skip the argument value
			Settings::getInstance()->setInt("ScreenRotate", rotate);
		}else if(strcmp(argv[i], "--gamelist-only") == 0)
		{
			Settings::getInstance()->setBool("ParseGamelistOnly", true);
		}else if(strcmp(argv[i], "--ignore-gamelist") == 0)
		{
			Settings::getInstance()->setBool("IgnoreGamelist", true);
		}else if(strcmp(argv[i], "--show-hidden-files") == 0)
		{
			Settings::getInstance()->setBool("ShowHiddenFiles", true);
		}else if(strcmp(argv[i], "--draw-framerate") == 0)
		{
			Settings::getInstance()->setBool("DrawFramerate", true);
		}else if(strcmp(argv[i], "--no-exit") == 0)
		{
			Settings::getInstance()->setBool("ShowExit", false);
		}else if(strcmp(argv[i], "--no-confirm-quit") == 0)
		{
			Settings::getInstance()->setBool("ConfirmQuit", false);
		}else if(strcmp(argv[i], "--no-splash") == 0)
		{
			Settings::getInstance()->setBool("SplashScreen", false);
		}else if(strcmp(argv[i], "--debug") == 0)
		{
			Settings::getInstance()->setBool("Debug", true);
			Settings::getInstance()->setBool("HideConsole", false);
			Log::setReportingLevel(LogDebug);
		}else if(strcmp(argv[i], "--fullscreen-borderless") == 0)
		{
			Settings::getInstance()->setBool("FullscreenBorderless", true);
		}else if(strcmp(argv[i], "--windowed") == 0)
		{
			Settings::getInstance()->setBool("Windowed", true);
		}else if(strcmp(argv[i], "--vsync") == 0)
		{
			bool vsync = (strcmp(argv[i + 1], "on") == 0 || strcmp(argv[i + 1], "1") == 0) ? true : false;
			Settings::getInstance()->setBool("VSync", vsync);
			i++; // skip vsync value
		}else if(strcmp(argv[i], "--scrape") == 0)
		{
			scrape_cmdline = true;
		}else if(strcmp(argv[i], "--max-vram") == 0)
		{
			int maxVRAM = atoi(argv[i + 1]);
			Settings::getInstance()->setInt("MaxVRAM", maxVRAM);
		}
		else if (strcmp(argv[i], "--force-kiosk") == 0)
		{
			Settings::getInstance()->setBool("ForceKiosk", true);
		}
		else if (strcmp(argv[i], "--force-kid") == 0)
		{
			Settings::getInstance()->setBool("ForceKid", true);
		}
		else if (strcmp(argv[i], "--force-disable-filters") == 0)
		{
			Settings::getInstance()->setBool("ForceDisableFilters", true);
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
		{
#ifdef WIN32
			// This is a bit of a hack, but otherwise output will go to nowhere
			// when the application is compiled with the "WINDOWS" subsystem (which we usually are).
			// If you're an experienced Windows programmer and know how to do this
			// the right way, please submit a pull request!
			AttachConsole(ATTACH_PARENT_PROCESS);
			freopen("CONOUT$", "wb", stdout);
#endif
			std::cout <<
				"EmulationStation, a graphical front-end for ROM browsing.\n"
				"Written by Alec \"Aloshi\" Lofquist.\n"
				"Version " << PROGRAM_VERSION_STRING << ", built " << PROGRAM_BUILT_STRING << "\n"
				"Command line arguments:\n"
				"\nGeometry settings:\n"
				"--resolution WIDTH HEIGHT      try and force a particular resolution\n"
				"--screenrotate N               rotate a quarter turn clockwise for each N\n"
				"--screensize WIDTH HEIGHT      for a canvas smaller than the full resolution,\n"
				"                               or if rotating into portrait mode\n"
				"--screenoffset X Y             move the canvas by x,y pixels\n"
				"--fullscreen-borderless        borderless fullscreen window\n"
				"--windowed                     not fullscreen, should be used with --resolution\n"
				"--monitor N                    monitor index (0-)\n"
				"\nGame and settings visibility in ES and behaviour of ES:\n"
				"--force-disable-filters        force the UI to ignore applied filters on\n"
				"                               gamelist (p)\n"
				"--force-kid                    force the UI mode to be Kid\n"
				"--force-kiosk                  force the UI mode to be Kiosk\n"
				"--no-confirm-quit              omit confirm dialog on actions of quit menu\n"
				"--no-exit                      don't show the exit option in the menu\n"
				"--no-splash                    don't show the splash screen\n"
				"\nGamelist related:\n"
				"--gamelist-only                use gamelist.xml as trusted source and do not\n"
				"                               check any path entries of gamelist.xml (p)\n"
				"--ignore-gamelist              do not read gamelist.xml files (useful for\n"
				"                               troubleshooting)\n"
				"\nAdvanced settings:\n"
				"--debug                        more logging, show console on Windows. Enables\n"
				"                               these keyboard shortcuts with left CTRL-key:\n"
				"                               +G: Toggle Gridlayout boundary boxes\n"
				"                               +I: Toggle image boundary box\n"
				"                               +R: Reload all UI views (theme, gamelist, system)\n"
				"                               +T: Toggle textcomponent boundary box\n"
				"--draw-framerate               display the framerate (p)\n"
				"--max-vram SIZE                maximum VRAM to use in MB before swapping,\n"
				"                               use 0 for unlimited (p)\n"
				"--show-hidden-files            show also hidden files of filesystem, no effect\n"
				"                               if --gamelist-only is also set (p)\n"
				"--vsync 1|0                    turn vsync on (1) or off (0) (default is on)\n"
				"\nGeneric switches:\n"
				"--help, -h                     summon a sentient, angry tuba\n\n"
				"--home PATH                    directory to use as home folder for\n"
				"                               .emulationstation/es_settings.cfg, aso.\n"
				"                               Subfolder .emulationstation/ will be created.\n"
				"\nScrape mode:\n"
				"--scrape                       scrape using command line interface\n\n"
				"Note: Switches marked (p) will be persisted in es_settings.cfg when any\n"
				"setting is changed via EmulationStation UI.\n\n"
				"Please refer to the online documentation for additional information:\n"
				"https://retropie.org.uk/docs/EmulationStation/\n";
			return false; //exit after printing help
		}
	}

	return true;
}

bool verifyHomeFolderExists()
{
	//make sure the config directory exists
	std::string home = Utils::FileSystem::getHomePath();
	std::string configDir = home + "/.emulationstation";
	if(!Utils::FileSystem::exists(configDir))
	{
		std::cout << "Creating config directory \"" << configDir << "\"\n";
		Utils::FileSystem::createDirectory(configDir);
		if(!Utils::FileSystem::exists(configDir))
		{
			std::cerr << "Config directory could not be created!\n";
			return false;
		}
	}

	return true;
}

// Returns true if everything is OK,
bool loadSystemConfigFile(Window* window, const char** errorString)
{
	*errorString = NULL;

	if(!SystemData::loadConfig(window))
	{
		LOG(LogError) << "Error while parsing systems configuration file!";
		*errorString = "IT LOOKS LIKE YOUR SYSTEMS CONFIGURATION FILE HAS NOT BEEN SET UP OR IS INVALID. YOU'LL NEED TO DO THIS BY HAND, UNFORTUNATELY.\n\n"
			"VISIT EMULATIONSTATION.ORG FOR MORE INFORMATION.";
		return false;
	}

	if(SystemData::sSystemVector.size() == 0)
	{
		LOG(LogError) << "No systems found! Does at least one system have a game present? (check that extensions match!)\n(Also, make sure you've updated your es_systems.cfg for XML!)";
		*errorString = "WE CAN'T FIND ANY SYSTEMS!\n"
			"CHECK THAT YOUR PATHS ARE CORRECT IN THE SYSTEMS CONFIGURATION FILE, "
			"AND YOUR GAME DIRECTORY HAS AT LEAST ONE GAME WITH THE CORRECT EXTENSION.\n\n"
			"VISIT EMULATIONSTATION.ORG FOR MORE INFORMATION.";
		return false;
	}

	return true;
}

//called on exit, assuming we get far enough to have the log initialized
void onExit()
{
	Log::close();
}

namespace
{
	struct ScriptedPress
	{
		int atMs;
		Uint8 button;
		Uint8 state;
	};

	std::vector<ScriptedPress> parseScript(const char* text)
	{
		static const std::map<std::string, Uint8> buttons = {
			{ "a", 0 }, { "b", 1 }, { "x", 2 }, { "y", 3 }, { "select", 8 }, { "start", 9 },
			{ "left", 11 }, { "right", 12 }, { "up", 13 }, { "down", 14 } };
		constexpr int tapMs = 100;

		std::vector<ScriptedPress> script;
		if(text == nullptr)
			return script;

		std::istringstream stream(text);
		std::string item;
		while(std::getline(stream, item, ','))
		{
			const std::string::size_type colon = item.find(':');
			if(colon == std::string::npos)
				continue;

			const int at = std::atoi(item.substr(0, colon).c_str());
			std::string name = item.substr(colon + 1);
			const char mode = (!name.empty() && (name[0] == '+' || name[0] == '-')) ? name[0] : 't';
			if(mode != 't')
				name.erase(0, 1);

			const auto button = buttons.find(name);
			if(button == buttons.end())
				continue;

			if(mode != '-')
				script.push_back({ at, button->second, SDL_PRESSED });
			if(mode != '+')
				script.push_back({ mode == 't' ? at + tapMs : at, button->second, SDL_RELEASED });
		}

		std::stable_sort(script.begin(), script.end(),
			[](const ScriptedPress& left, const ScriptedPress& right) { return left.atMs < right.atMs; });
		return script;
	}

	void pushPress(const ScriptedPress& press)
	{
		SDL_Event event{};
		event.type = press.state == SDL_PRESSED ? SDL_JOYBUTTONDOWN : SDL_JOYBUTTONUP;
		event.jbutton.which = 0;
		event.jbutton.button = press.button;
		event.jbutton.state = press.state;
		SDL_PushEvent(&event);
	}
}

namespace
{
	void logCrash(int signal, siginfo_t* info, void* context)
	{
		const ucontext_t* uc = static_cast<const ucontext_t*>(context);
		const int fd = open("/var/lib/hakchi/es/crash.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if(fd >= 0)
		{
			char line[160];
			int length = snprintf(line, sizeof line, "signal %d fault %p pc %08lx lr %08lx sp %08lx\n", signal, info->si_addr,
				uc->uc_mcontext.arm_pc, uc->uc_mcontext.arm_lr, uc->uc_mcontext.arm_sp);
			static_cast<void>(write(fd, line, static_cast<size_t>(length)));

			const unsigned long* stack = reinterpret_cast<const unsigned long*>(uc->uc_mcontext.arm_sp);
			for(int word = 0; word < 256; ++word)
			{
				if(stack[word] >= 0x8000 && stack[word] < 0x1000000)
				{
					length = snprintf(line, sizeof line, "stack %08lx\n", stack[word]);
					static_cast<void>(write(fd, line, static_cast<size_t>(length)));
				}
			}
			close(fd);
		}
		_exit(128 + signal);
	}

	void exitOnTerminate(int signal)
	{
		_exit(128 + signal);
	}

	void installTerminateHandler()
	{
		struct sigaction action;
		action.sa_handler = exitOnTerminate;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		for(const int signal : { SIGTERM, SIGHUP })
			sigaction(signal, &action, nullptr);
	}

	void installCrashLogger()
	{
		struct sigaction action;
		action.sa_sigaction = logCrash;
		sigemptyset(&action.sa_mask);
		action.sa_flags = SA_SIGINFO;
		for(const int signal : { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT })
			sigaction(signal, &action, nullptr);
	}
}

int main(int argc, char* argv[])
{
	installCrashLogger();
	std::locale::global(std::locale("C"));

	if(!parseArgs(argc, argv))
		return 0;

	// only show the console on Windows if HideConsole is false
#ifdef WIN32
	// MSVC has a "SubSystem" option, with two primary options: "WINDOWS" and "CONSOLE".
	// In "WINDOWS" mode, no console is automatically created for us.  This is good,
	// because we can choose to only create the console window if the user explicitly
	// asks for it, preventing it from flashing open and then closing.
	// In "CONSOLE" mode, a console is always automatically created for us before we
	// enter main. In this case, we can only hide the console after the fact, which
	// will leave a brief flash.
	// TL;DR: You should compile ES under the "WINDOWS" subsystem.
	// I have no idea how this works with non-MSVC compilers.
	if(!Settings::getInstance()->getBool("HideConsole"))
	{
		// we want to show the console
		// if we're compiled in "CONSOLE" mode, this is already done.
		// if we're compiled in "WINDOWS" mode, no console is created for us automatically;
		// the user asked for one, so make one and then hook stdin/stdout/sterr up to it
		if(AllocConsole()) // should only pass in "WINDOWS" mode
		{
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "wb", stdout);
			freopen("CONOUT$", "wb", stderr);
		}
	}else{
		// we want to hide the console
		// if we're compiled with the "WINDOWS" subsystem, this is already done.
		// if we're compiled with the "CONSOLE" subsystem, a console is already created;
		// it'll flash open, but we hide it nearly immediately
		if(GetConsoleWindow()) // should only pass in "CONSOLE" mode
			ShowWindow(GetConsoleWindow(), SW_HIDE);
	}
#endif

	// call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
#endif

	//if ~/.emulationstation doesn't exist and cannot be created, bail
	if(!verifyHomeFolderExists())
		return 1;

	//start the logger
	Log::init();
	Log::open();
	LOG(LogInfo) << "EmulationStation - v" << PROGRAM_VERSION_STRING << ", built " << PROGRAM_BUILT_STRING;

	//always close the log on exit
	atexit(&onExit);

	MenuColors::apply(MenuColors::schemeFromName(Settings::getInstance()->getString("ColorScheme")));

	Window window;
	SystemScreenSaver screensaver(&window);
	PowerSaver::init();
	ViewController::init(&window);
	CollectionSystemManager::init(&window);
	MameNames::init();
	window.pushGui(ViewController::get());

	bool splashScreen = Settings::getInstance()->getBool("SplashScreen");

	if(!scrape_cmdline)
	{
		if(!window.init())
		{
			LOG(LogError) << "Window failed to initialize!";
			return 1;
		}

		AudioManager::getInstance();
		installTerminateHandler();

		if (splashScreen)
		{
			std::string progressText = "Loading system config...";
			window.renderLoadingScreen(progressText);
		}
	}

	const char* errorMsg = NULL;
	if(!loadSystemConfigFile(splashScreen ? &window : nullptr, &errorMsg))
	{
		// something went terribly wrong
		if(errorMsg == NULL)
		{
			LOG(LogError) << "Unknown error occured while parsing system config file.";
			if(!scrape_cmdline)
				Renderer::deinit();
			return 1;
		}

		// we can't handle es_systems.cfg file problems inside ES itself, so display the error message then quit
		LOG(LogWarning) << "Startup dialog: " << errorMsg;
		window.pushGui(new GuiMsgBox(&window,
			errorMsg,
			"QUIT", [] {
				SDL_Event* quit = new SDL_Event();
				quit->type = SDL_QUIT;
				SDL_PushEvent(quit);
			}));
	}

	//run the command line scraper then quit
	if(scrape_cmdline)
	{
		return run_scraper_cmdline();
	}

	// preload what we can right away instead of waiting for the user to select it
	// this makes for no delays when accessing content, but a longer startup time
	ViewController::get()->preload();

	if(splashScreen)
		window.renderLoadingScreen("Done.");

	InputManager::getInstance()->init();

	if(errorMsg == NULL && Utils::FileSystem::exists(InputManager::getConfigPath()))
	{
		const Uint32 deadline = SDL_GetTicks() + 3000;
		while(InputManager::getInstance()->getNumConfiguredDevices() == 0 && !SDL_TICKS_PASSED(SDL_GetTicks(), deadline))
		{
			SDL_Event event;
			while(SDL_PollEvent(&event))
				InputManager::getInstance()->parseEvent(event, &window);
			SDL_Delay(50);
		}
	}

	//choose which GUI to open depending on if an input configuration already exists
	if(errorMsg == NULL)
	{
		if(Utils::FileSystem::exists(InputManager::getConfigPath()) && InputManager::getInstance()->getNumConfiguredDevices() > 0)
		{
			ViewController::get()->goToStart();
		}else{
			LOG(LogWarning) << "Startup dialog: controller configuration, input config file present: "
				<< Utils::FileSystem::exists(InputManager::getConfigPath())
				<< ", configured devices: " << InputManager::getInstance()->getNumConfiguredDevices();
			window.pushGui(new GuiDetectDevice(&window, true, [] { ViewController::get()->goToStart(); }));
		}
	}

	int lastTime = SDL_GetTicks();
	int ps_time = SDL_GetTicks();

	bool running = true;

	const bool bench = std::getenv("ES_BENCH") != nullptr;
	int benchFrames = 0;
	int benchWorst = 0;
	int benchReport = SDL_GetTicks();
	int benchStep = SDL_GetTicks();

	const std::vector<ScriptedPress> script = parseScript(std::getenv("ES_INJECT"));
	std::size_t scriptNext = 0;
	const int scriptStart = SDL_GetTicks();

	while(running)
	{
		SDL_Event event;
		bool ps_standby = PowerSaver::getState() && (int) SDL_GetTicks() - ps_time > PowerSaver::getMode();

		if(ps_standby ? SDL_WaitEventTimeout(&event, PowerSaver::getTimeout()) : SDL_PollEvent(&event))
		{
			do
			{
				InputManager::getInstance()->parseEvent(event, &window);

				if(event.type == SDL_QUIT)
					running = false;
			} while(SDL_PollEvent(&event));

			// triggered if exiting from SDL_WaitEvent due to event
			if (ps_standby)
				// show as if continuing from last event
				lastTime = SDL_GetTicks();

			// reset counter
			ps_time = SDL_GetTicks();
		}
		else if (ps_standby)
		{
			// If exitting SDL_WaitEventTimeout due to timeout. Trail considering
			// timeout as an event
			ps_time = SDL_GetTicks();
		}

		if(window.isSleeping())
		{
			lastTime = SDL_GetTicks();
			SDL_Delay(1); // this doesn't need to be accurate, we're just giving up our CPU time until something wakes us up
			continue;
		}

		int curTime = SDL_GetTicks();
		int deltaTime = curTime - lastTime;
		lastTime = curTime;

		// cap deltaTime if it ever goes negative
		if(deltaTime < 0)
			deltaTime = 1000;

		window.update(deltaTime);
		window.render();
		Renderer::swapBuffers();

		while(scriptNext < script.size() && curTime - scriptStart >= script[scriptNext].atMs)
			pushPress(script[scriptNext++]);

		if(bench)
		{
			++benchFrames;
			benchWorst = std::max(benchWorst, deltaTime);

			if(curTime - benchStep >= 600)
			{
				benchStep = curTime;
				for(const Uint8 state : { SDL_PRESSED, SDL_RELEASED })
				{
					SDL_Event press{};
					press.type = state == SDL_PRESSED ? SDL_JOYBUTTONDOWN : SDL_JOYBUTTONUP;
					press.jbutton.which = 0;
					press.jbutton.button = 12;
					press.jbutton.state = state;
					SDL_PushEvent(&press);
				}
			}

			if(curTime - benchReport >= 2000)
			{
				std::cerr << "bench: " << benchFrames * 1000 / (curTime - benchReport) << " fps, worst frame " << benchWorst << " ms\n";
				benchFrames = 0;
				benchWorst = 0;
				benchReport = curTime;
			}
		}

		Log::flush();
	}

	while(window.peekGui() != ViewController::get())
		delete window.peekGui();

	InputManager::getInstance()->deinit();
	window.deinit();

	// Join the VLC cleanup worker and release the VLC instance. Must happen after
	// window.deinit() so all VideoVlcComponents are destroyed and their final
	// stopVideo() cleanup tasks are already posted to the queue.
	VideoVlcComponent::deinit();

	MameNames::deinit();
	CollectionSystemManager::deinit();
	waitForGamelistWrites();
	SystemData::deleteSystems();

	// call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
	FreeImage_DeInitialise();
#endif

	processQuitMode();

	ProfileDump();

	LOG(LogInfo) << "EmulationStation cleanly shutting down.";

	return 0;
}
