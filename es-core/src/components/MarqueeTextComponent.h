/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_CORE_COMPONENTS_MARQUEE_TEXT_COMPONENT_H
#define ES_CORE_COMPONENTS_MARQUEE_TEXT_COMPONENT_H

#include "GuiComponent.h"
#include "components/TextComponent.h"

class MarqueeTextComponent : public GuiComponent
{
public:
	explicit MarqueeTextComponent(Window* window);

	void setText(const std::string& text);
	void setColor(unsigned int color) { mText.setColor(color); }
	void setFont(const std::shared_ptr<Font>& font) { mText.setFont(font); }
	void setHorizontalAlignment(Alignment alignment) { mText.setHorizontalAlignment(alignment); }

	std::string getValue() const override { return mText.getValue(); }
	void setValue(const std::string& value) override { setText(value); }

	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	enum class Phase { Waiting, Scrolling, Holding };

	void restart();

	TextComponent mText;
	Vector2f mBox;
	Phase mPhase = Phase::Waiting;
	int mPhaseMs = 0;
	float mOffset = 0.0f;
};

#endif // ES_CORE_COMPONENTS_MARQUEE_TEXT_COMPONENT_H
