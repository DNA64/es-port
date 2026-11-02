/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_CORE_COMPONENTS_PAGED_TEXT_COMPONENT_H
#define ES_CORE_COMPONENTS_PAGED_TEXT_COMPONENT_H

#include "GuiComponent.h"
#include "components/TextComponent.h"

class PagedTextComponent : public GuiComponent
{
public:
	explicit PagedTextComponent(Window* window);

	void setText(const std::string& text);
	void setColor(unsigned int color) { mText.setColor(color); }
	void setFont(const std::shared_ptr<Font>& font) { mText.setFont(font); }
	void restart();

	std::string getValue() const override { return mText.getValue(); }
	void setValue(const std::string& value) override { setText(value); }

	void onSizeChanged() override;
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	void layout();
	bool scrolls() const;
	float cycleHeight() const;

	TextComponent mText;
	Vector2f mBox;
	float mLineHeight = 0.0f;
	int mVisibleLines = 1;
	int mHoldMs = 0;
	float mOffset = 0.0f;
};

#endif // ES_CORE_COMPONENTS_PAGED_TEXT_COMPONENT_H
