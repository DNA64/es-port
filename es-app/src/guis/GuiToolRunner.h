/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_GUIS_GUI_TOOL_RUNNER_H
#define ES_APP_GUIS_GUI_TOOL_RUNNER_H

#include "components/BusyComponent.h"
#include "GuiComponent.h"
#include <functional>
#include <future>
#include <string>

class GuiToolRunner : public GuiComponent
{
public:
	GuiToolRunner(Window* window, std::function<std::string()> work, std::function<void(const std::string&)> done = nullptr);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	BusyComponent mBusy;
	std::future<std::string> mResult;
	std::function<void(const std::string&)> mDone;
};

#endif // ES_APP_GUIS_GUI_TOOL_RUNNER_H
