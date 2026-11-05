/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

namespace
{
	constexpr std::size_t KeyBytes = KEY_MAX / 8 + 1;
	constexpr const char* DefaultDevice = "Clovercon";

	class Device
	{
	public:
		explicit Device(const std::string& path) : mFd(::open(path.c_str(), O_RDONLY | O_NONBLOCK)) {}
		~Device() { if (mFd >= 0) ::close(mFd); }
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		bool isOpen() const { return mFd >= 0; }

		std::string name() const
		{
			std::array<char, 256> buffer;
			buffer.fill(0);
			return ::ioctl(mFd, EVIOCGNAME(buffer.size() - 1), buffer.data()) >= 0 ? std::string(buffer.data()) : std::string();
		}

		std::array<unsigned char, KeyBytes> keys() const
		{
			std::array<unsigned char, KeyBytes> state;
			state.fill(0);
			static_cast<void>(::ioctl(mFd, EVIOCGKEY(state.size()), state.data()));
			return state;
		}

	private:
		int mFd;
	};

	struct Button
	{
		const char* name;
		int code;
	};

	const std::vector<Button>& buttons()
	{
		static const std::vector<Button> table = {
			{ "select", BTN_SELECT }, { "start", BTN_START }, { "a", BTN_A }, { "b", BTN_B }, { "x", BTN_X },
			{ "y", BTN_Y }, { "l", BTN_TL }, { "r", BTN_TR }, { "mode", BTN_MODE } };
		return table;
	}

	bool isDown(const std::array<unsigned char, KeyBytes>& keys, int code)
	{
		return (keys[static_cast<std::size_t>(code) / 8] >> (code % 8)) & 1;
	}

	std::vector<std::string> deviceNodes()
	{
		std::vector<std::string> nodes;
		if (DIR* dir = ::opendir("/dev/input"))
		{
			while (const dirent* entry = ::readdir(dir))
				if (std::strncmp(entry->d_name, "event", 5) == 0)
					nodes.push_back(std::string("/dev/input/") + entry->d_name);
			::closedir(dir);
		}
		std::sort(nodes.begin(), nodes.end());
		return nodes;
	}
}

int main(int argc, char* argv[])
{
	bool dump = false;
	std::string match = DefaultDevice;
	std::vector<std::string> wanted;

	for (int index = 1; index < argc; ++index)
	{
		const std::string argument = argv[index];
		if (argument == "--dump")
			dump = true;
		else if (argument == "--device" && index + 1 < argc)
			match = argv[++index];
		else
			wanted.push_back(argument);
	}

	bool everyWantedDown = !wanted.empty();
	bool found = false;
	for (const std::string& path : deviceNodes())
	{
		const Device device(path);
		if (!device.isOpen() || device.name().find(match) == std::string::npos)
			continue;

		found = true;
		const auto keys = device.keys();
		if (dump)
		{
			std::cout << path << " \"" << device.name() << "\" down:";
			for (int code = 0; code < KEY_MAX; ++code)
				if (isDown(keys, code))
					std::cout << ' ' << code;
			std::cout << '\n';
			continue;
		}

		for (const std::string& name : wanted)
		{
			const auto button = std::find_if(buttons().begin(), buttons().end(), [&name](const Button& entry) { return name == entry.name; });
			if (button == buttons().end() || !isDown(keys, button->code))
				everyWantedDown = false;
		}
		if (everyWantedDown)
			return 0;
		everyWantedDown = !wanted.empty();
	}
	if (dump)
		return 0;
	return found ? 1 : 2;
}
