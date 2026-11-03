/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiAbout.h"

#include "MenuColors.h"
#include "InputConfig.h"
#include "EmulationStation.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"
#include "utils/StringUtil.h"

GuiAbout::GuiAbout(Window* window) : GuiComponent(window),
	mFrame(window, ":/frame.png"), mLogo(window), mVersion(window), mCredit(window), mControls(window)
{
	const float screenWidth = static_cast<float>(Renderer::getScreenWidth());
	const float screenHeight = static_cast<float>(Renderer::getScreenHeight());
	const float width = screenWidth * 0.55f;
	const float margin = width * 0.06f;
	const float textWidth = width - 2 * margin;

	mFrame.setCenterColor(MenuColors::Panel);
	mFrame.setEdgeColor(MenuColors::Panel);

	mLogo.setImage(":/defkorns_logo.png");
	mLogo.setMaxSize(textWidth, screenHeight * 0.36f);
	mLogo.setOrigin(0.5f, 0.0f);

	const auto style = [textWidth](TextComponent& text, const std::string& value, const std::shared_ptr<Font>& font, unsigned int color) {
		text.setFont(font);
		text.setColor(color);
		text.setHorizontalAlignment(ALIGN_CENTER);
		text.setText(value);
		text.setSize(textWidth, 0.0f);
	};
	style(mVersion, "EMULATIONSTATION V" + Utils::String::toUpper(PROGRAM_VERSION_STRING), Font::get(FONT_SIZE_MENU), MenuColors::Title);
	style(mCredit, "SNES CLASSIC PORT BY DEFKORNS", Font::get(FONT_SIZE_SMALL), MenuColors::Dim);
	style(mControls, "UP: MENU   DOWN: SUSPEND POINT LIST\nA: SELECT   B: GO TO SAVED GAME   Y: SORT", Font::get(FONT_SIZE_SMALL), MenuColors::Text);

	const float gap = screenHeight * 0.02f;
	const float logoHeight = mLogo.getSize().y();
	const float height = margin + logoHeight + gap + mVersion.getSize().y() + mCredit.getSize().y() + gap * 2 + mControls.getSize().y() + margin;

	setSize(width, height);
	setPosition((screenWidth - width) / 2.0f, (screenHeight - height) / 2.0f);
	mFrame.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	float y = margin;
	mLogo.setPosition(width / 2.0f, y);
	y += logoHeight + gap;
	mVersion.setPosition(margin, y);
	y += mVersion.getSize().y();
	mCredit.setPosition(margin, y);
	y += mCredit.getSize().y() + gap * 2;
	mControls.setPosition(margin, y);

	addChild(&mFrame);
	addChild(&mLogo);
	addChild(&mVersion);
	addChild(&mCredit);
	addChild(&mControls);
}

bool GuiAbout::input(InputConfig* config, Input input)
{
	if(input.value != 0 && (config->isMappedTo("a", input) || config->isMappedTo("b", input)))
	{
		delete this;
		return true;
	}
	return true;
}

std::vector<HelpPrompt> GuiAbout::getHelpPrompts()
{
	return { HelpPrompt("a", "back"), HelpPrompt("b", "back") };
}
