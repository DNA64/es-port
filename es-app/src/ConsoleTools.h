/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_CONSOLE_TOOLS_H
#define ES_APP_CONSOLE_TOOLS_H

#include <map>
#include <string>

namespace ConsoleTools
{
	std::string run(const std::string& arguments, const std::map<std::string, std::string>& environment = {});
	bool isOn(const std::string& arguments);
}

#endif // ES_APP_CONSOLE_TOOLS_H
