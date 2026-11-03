/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiSystemInfo.h"

#include "InputConfig.h"
#include "MenuColors.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"
#include <algorithm>
#include <sstream>

namespace
{
	constexpr unsigned int BarBackground = 0x121212FF;
	constexpr unsigned int BarFill = 0x8A8A8AFF;
	constexpr unsigned int BarFull = 0xB8504DFF;
	constexpr int BarWarningPercent = 90;

	struct Row
	{
		std::string label;
		std::string value;
		int percent = -1;
	};

	std::vector<Row> parseReport(const std::string& report)
	{
		std::vector<Row> rows;
		std::istringstream lines(report);
		std::string line;
		while(std::getline(lines, line))
		{
			if(!line.empty() && line.back() == '\r')
				line.pop_back();
			const std::size_t first = line.find('\t');
			if(first == std::string::npos)
				continue;
			Row row;
			row.label = line.substr(0, first);
			const std::size_t second = line.find('\t', first + 1);
			row.value = line.substr(first + 1, second == std::string::npos ? std::string::npos : second - first - 1);
			if(second != std::string::npos)
				row.percent = std::max(0, std::min(100, std::atoi(line.c_str() + second + 1)));
			rows.push_back(row);
		}
		return rows;
	}
}

GuiSystemInfo::GuiSystemInfo(Window* window, const std::string& report) : GuiComponent(window),
	mFrame(window, ":/frame.png"), mTitle(window, "SYSTEM INFORMATION", Font::get(FONT_SIZE_MENU_TITLE), MenuColors::Title, ALIGN_CENTER)
{
	const float screenWidth = static_cast<float>(Renderer::getScreenWidth());
	const float screenHeight = static_cast<float>(Renderer::getScreenHeight());
	const float width = screenWidth * 0.52f;
	const float margin = width * 0.07f;
	const float innerWidth = width - 2 * margin;
	const std::shared_ptr<Font> font = Font::get(FONT_SIZE_MENU);
	const float lineHeight = font->getHeight();
	const float rowGap = lineHeight * 0.28f;

	mBarHeight = std::max(4.0f, screenHeight * 0.011f);
	mBarX = margin;
	mBarWidth = innerWidth;

	mFrame.setCenterColor(MenuColors::Panel);
	mFrame.setEdgeColor(MenuColors::Panel);

	float y = margin * 0.7f;
	mTitle.setSize(innerWidth, 0.0f);
	mTitle.setPosition(margin, y);
	y += mTitle.getSize().y() + rowGap * 1.5f;

	for(const Row& row : parseReport(report))
	{
		auto label = std::make_shared<TextComponent>(window, row.label, font, MenuColors::Dim, ALIGN_LEFT);
		label->setPosition(margin, y);
		label->setSize(innerWidth, lineHeight);
		auto value = std::make_shared<TextComponent>(window, row.value, font, MenuColors::Text, ALIGN_RIGHT);
		value->setPosition(margin, y);
		value->setSize(innerWidth, lineHeight);
		mTexts.push_back(std::move(label));
		mTexts.push_back(std::move(value));
		y += lineHeight;
		if(row.percent >= 0)
		{
			mBars.push_back({ y + rowGap * 0.2f, row.percent });
			y += mBarHeight + rowGap * 0.6f;
		}
		y += rowGap;
	}

	const float height = y + margin * 0.5f;
	setSize(width, height);
	setPosition((screenWidth - width) / 2.0f, (screenHeight - height) / 2.0f);
	mFrame.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	addChild(&mFrame);
	addChild(&mTitle);
	for(auto& text : mTexts)
		addChild(text.get());
}

bool GuiSystemInfo::input(InputConfig* config, Input input)
{
	if(input.value != 0 && (config->isMappedTo("a", input) || config->isMappedTo("b", input)))
	{
		delete this;
		return true;
	}
	return true;
}

void GuiSystemInfo::render(const Transform4x4f& parentTrans)
{
	GuiComponent::render(parentTrans);

	const Transform4x4f trans = parentTrans * getTransform();
	for(const Bar& bar : mBars)
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(mBarX, bar.y, mBarWidth, mBarHeight, BarBackground, BarBackground);
		const unsigned int color = bar.percent >= BarWarningPercent ? BarFull : BarFill;
		Renderer::setMatrix(trans);
		Renderer::drawRect(mBarX, bar.y, mBarWidth * static_cast<float>(bar.percent) / 100.0f, mBarHeight, color, color);
	}
}

std::vector<HelpPrompt> GuiSystemInfo::getHelpPrompts()
{
	return { HelpPrompt("a", "back"), HelpPrompt("b", "back") };
}
