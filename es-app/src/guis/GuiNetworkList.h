/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_GUIS_GUI_NETWORK_LIST_H
#define ES_APP_GUIS_GUI_NETWORK_LIST_H

#include "components/MenuComponent.h"
#include "GuiComponent.h"
#include <string>
#include <vector>

struct WifiNetwork
{
	std::string name;
	bool secured;
};

class GuiNetworkList : public GuiComponent
{
public:
	GuiNetworkList(Window* window, const std::vector<WifiNetwork>& networks);

	static std::vector<WifiNetwork> parse(const std::string& scanOutput);

	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	HelpStyle getHelpStyle() override;

private:
	void addNetworkRow(const std::string& label, const std::string& detail, const std::function<void()>& action);
	void askPassword(const std::string& name);
	void askHiddenName();
	void connect(const std::string& name, const std::string& password);

	MenuComponent mMenu;
};

#endif // ES_APP_GUIS_GUI_NETWORK_LIST_H
