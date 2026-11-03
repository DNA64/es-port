/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "MenuColors.h"

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

	namespace { Scheme current = Scheme::Dark; }

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
}
