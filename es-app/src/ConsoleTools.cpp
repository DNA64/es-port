/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "ConsoleTools.h"

#include <array>
#include <cstdio>
#include <memory>

namespace
{
	constexpr const char* Script = "/var/lib/hakchi/es/es-tools.sh";

	std::string shellQuote(const std::string& value)
	{
		std::string quoted = "'";
		for(const char c : value)
		{
			if(c == '\'')
				quoted += "'\\''";
			else
				quoted += c;
		}
		return quoted + "'";
	}
}

namespace ConsoleTools
{
	std::string run(const std::string& arguments, const std::map<std::string, std::string>& environment)
	{
		std::string command;
		for(const auto& entry : environment)
			command += entry.first + "=" + shellQuote(entry.second) + " ";
		command += std::string("sh ") + Script + " " + arguments + " 2>&1";
		const std::unique_ptr<FILE, int (*)(FILE*)> pipe(popen(command.c_str(), "r"), pclose);
		if(!pipe)
			return "";

		std::string output;
		std::array<char, 256> buffer;
		while(std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
			output += buffer.data();

		while(!output.empty() && (output.back() == '\n' || output.back() == '\r' || output.back() == ' '))
			output.pop_back();
		return output;
	}

	bool isOn(const std::string& arguments)
	{
		return run(arguments) == "on";
	}
}
