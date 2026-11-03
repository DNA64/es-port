/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "MenuColors.h"

#include "Settings.h"
#include <cstdlib>

namespace MenuColors
{
	namespace
	{
		struct Palette
		{
			unsigned int panel, field, text, title, dim, separator, selector, selectorEdge, buttonFocused, disabled, icon, key, keyHighlight, keyHighlightText;
		};

		constexpr Palette DarkPalette = { 0x1E1E1EFF, 0x121212FF, 0xF0F0F0FF, 0xBDBDBDFF, 0x9A9A9AFF, 0x353535FF, 0x444444FF, 0x8A8A8AFF, 0x707070FF, 0x6A2A2AFF, 0xFFFFFFFF, 0x2E2E2EFF, 0xE0E0E0FF, 0x121212FF };
		constexpr Palette LightPalette = { 0xFFFFFFFF, 0xF4F4F4FF, 0x777777FF, 0x555555FF, 0x999999FF, 0xC6C7C6FF, 0xDDDDDDFF, 0x878787FF, 0xFFFFFFFF, 0x770000FF, 0x777777FF, 0xEBEBEBFF, 0x555555FF, 0xFFFFFFFF };
	}

	unsigned int Panel = DarkPalette.panel;
	unsigned int Field = DarkPalette.field;
	unsigned int Text = DarkPalette.text;
	unsigned int Title = DarkPalette.title;
	unsigned int Dim = DarkPalette.dim;
	unsigned int Separator = DarkPalette.separator;
	unsigned int Selector = DarkPalette.selector;
	unsigned int SelectorEdge = DarkPalette.selectorEdge;
	unsigned int ButtonFocused = DarkPalette.buttonFocused;
	unsigned int Disabled = DarkPalette.disabled;
	unsigned int Icon = DarkPalette.icon;
	unsigned int Key = DarkPalette.key;
	unsigned int KeyHighlight = DarkPalette.keyHighlight;
	unsigned int KeyHighlightText = DarkPalette.keyHighlightText;

	namespace
	{
		Scheme current = Scheme::Dark;

		bool parseColor(const std::string& text, unsigned int& out)
		{
			if(text.size() != 6 && text.size() != 8)
				return false;
			char* end = nullptr;
			const unsigned long value = std::strtoul(text.c_str(), &end, 16);
			if(end == nullptr || *end != '\0')
				return false;
			out = text.size() == 6 ? static_cast<unsigned int>((value << 8) | 0xFF) : static_cast<unsigned int>(value);
			return true;
		}
	}

	bool isLight() { return current == Scheme::Light; }

	Scheme schemeFromName(const std::string& name)
	{
		return name == "light" ? Scheme::Light : Scheme::Dark;
	}

	void apply(Scheme scheme)
	{
		current = scheme;
		const Palette& palette = scheme == Scheme::Light ? LightPalette : DarkPalette;
		Panel = palette.panel;
		Field = palette.field;
		Text = palette.text;
		Title = palette.title;
		Dim = palette.dim;
		Separator = palette.separator;
		Selector = palette.selector;
		SelectorEdge = palette.selectorEdge;
		ButtonFocused = palette.buttonFocused;
		Disabled = palette.disabled;
		Icon = palette.icon;
		Key = palette.key;
		KeyHighlight = palette.keyHighlight;
		KeyHighlightText = palette.keyHighlightText;
	}

	void applyOverrides(const std::function<std::string(const std::string&)>& variable)
	{
		const std::string scheme = variable("es_scheme");
		apply(scheme.empty() ? schemeFromName(Settings::getInstance()->getString("ColorScheme")) : schemeFromName(scheme));

		struct Entry { const char* name; unsigned int* target; };
		const Entry entries[] = {
			{ "es_panelColor", &Panel }, { "es_fieldColor", &Field }, { "es_textColor", &Text }, { "es_titleColor", &Title },
			{ "es_dimColor", &Dim }, { "es_separatorColor", &Separator }, { "es_selectorColor", &Selector },
			{ "es_selectorEdgeColor", &SelectorEdge }, { "es_buttonFocusedColor", &ButtonFocused },
			{ "es_disabledColor", &Disabled }, { "es_iconColor", &Icon }
		};
		for(const Entry& entry : entries)
		{
			unsigned int color = 0;
			if(parseColor(variable(entry.name), color))
				*entry.target = color;
		}
	}
}
