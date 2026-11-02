/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiNetworkList.h"

#include "guis/GuiToolRunner.h"
#include "guis/GuiVirtualKeyboard.h"
#include "views/ViewController.h"
#include "ConsoleTools.h"
#include "SystemData.h"
#include "Window.h"
#include "renderers/Renderer.h"
#include <sstream>

GuiNetworkList::GuiNetworkList(Window* window, const std::vector<WifiNetwork>& networks)
	: GuiComponent(window), mMenu(window, "NETWORKS")
{
	addChild(&mMenu);

	for(const WifiNetwork& network : networks)
	{
		const std::string name = network.name;
		const bool secured = network.secured;
		addNetworkRow(name, secured ? "PASSWORD" : "OPEN", [this, name, secured] {
			if(secured)
				askPassword(name);
			else
				connect(name, "");
		});
	}
	addNetworkRow("OTHER NETWORK...", "", [this] { askHiddenName(); });

	mMenu.addButton("BACK", "go back", [this] { delete this; });
	setSize(static_cast<float>(Renderer::getScreenWidth()), static_cast<float>(Renderer::getScreenHeight()));
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

std::vector<WifiNetwork> GuiNetworkList::parse(const std::string& scanOutput)
{
	std::vector<WifiNetwork> networks;
	std::istringstream lines(scanOutput);
	std::string line;
	while(std::getline(lines, line))
	{
		const std::size_t tab = line.rfind('\t');
		if(tab == std::string::npos || tab == 0)
			continue;
		networks.push_back({ line.substr(0, tab), line.substr(tab + 1) == "1" });
	}
	return networks;
}

void GuiNetworkList::addNetworkRow(const std::string& label, const std::string& detail, const std::function<void()>& action)
{
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, label, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	if(!detail.empty())
		row.addElement(std::make_shared<TextComponent>(mWindow, detail, Font::get(FONT_SIZE_SMALL), 0x999999FF), false);
	row.makeAcceptInputHandler(action);
	mMenu.addRow(row);
}

void GuiNetworkList::askPassword(const std::string& name)
{
	mWindow->pushGui(new GuiVirtualKeyboard(mWindow, "PASSWORD FOR " + name, "", true,
		[this, name](const std::string& password) {
			if(password.size() >= 8)
				connect(name, password);
		}));
}

void GuiNetworkList::askHiddenName()
{
	mWindow->pushGui(new GuiVirtualKeyboard(mWindow, "NETWORK NAME", "", false,
		[this](const std::string& name) {
			if(!name.empty())
				askPassword(name);
		}));
}

void GuiNetworkList::connect(const std::string& name, const std::string& password)
{
	mWindow->pushGui(new GuiToolRunner(mWindow, [name, password] {
		return ConsoleTools::run("wificonnect", { { "ES_SSID", name }, { "ES_PSK", password } });
	}));
}

bool GuiNetworkList::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("b", input) && input.value != 0)
	{
		delete this;
		return true;
	}

	return GuiComponent::input(config, input);
}

HelpStyle GuiNetworkList::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}

std::vector<HelpPrompt> GuiNetworkList::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
