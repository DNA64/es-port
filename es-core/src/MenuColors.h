/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_CORE_MENU_COLORS_H
#define ES_CORE_MENU_COLORS_H

#include <functional>
#include <string>

namespace MenuColors
{
	enum class Scheme { Dark, Light };

	extern unsigned int Panel;
	extern unsigned int Field;
	extern unsigned int Text;
	extern unsigned int Title;
	extern unsigned int Dim;
	extern unsigned int Separator;
	extern unsigned int Selector;
	extern unsigned int SelectorEdge;
	extern unsigned int ButtonFocused;
	extern unsigned int Disabled;
	extern unsigned int Icon;
	extern unsigned int Key;
	extern unsigned int KeyHighlight;
	extern unsigned int KeyHighlightText;

	bool isLight();
	Scheme schemeFromName(const std::string& name);
	void apply(Scheme scheme);
	void applyOverrides(const std::function<std::string(const std::string&)>& variable);
}

#endif // ES_CORE_MENU_COLORS_H
