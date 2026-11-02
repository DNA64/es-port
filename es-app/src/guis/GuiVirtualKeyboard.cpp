/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiVirtualKeyboard.h"

#include "InputConfig.h"
#include "Window.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"
#include <algorithm>
#include <array>

namespace
{
	constexpr std::size_t MaxSecretLength = 63;
	constexpr std::size_t MaxTextLength = 120;
	constexpr unsigned int PanelColor = 0x1F1D45FF;
	constexpr unsigned int FieldColor = 0x14122EFF;
	constexpr unsigned int KeyColor = 0x2B2960FF;
	constexpr unsigned int ActiveKeyColor = 0x4A43A8FF;
	constexpr unsigned int SelectedColor = 0x7B6FF5FF;
	constexpr unsigned int TextColor = 0xF4F1FFFF;
	constexpr unsigned int SelectedTextColor = 0x14122EFF;

	const std::array<const char*, 4> LowerRows = { "1234567890", "qwertyuiop", "asdfghjkl-", "zxcvbnm._@" };
	const std::array<const char*, 4> UpperRows = { "!#$%^&*()+", "QWERTYUIOP", "ASDFGHJKL_", "ZXCVBNM,;:" };

	struct SpecialLayout { int start; int width; };
	constexpr std::array<SpecialLayout, 6> SpecialLayouts = { {
		{ 0, 2 }, { 2, 1 }, { 3, 1 }, { 4, 2 }, { 6, 2 }, { 8, 2 }
	} };

	constexpr float PanelLeft = 0.10f;
	constexpr float PanelTop = 0.10f;
	constexpr float PanelWidth = 0.80f;
	constexpr float PanelHeight = 0.80f;
}

GuiVirtualKeyboard::GuiVirtualKeyboard(Window* window, const std::string& title, const std::string& initial, bool secret,
	std::function<void(const std::string&)> onAccept)
	: GuiComponent(window), mText(initial), mCursor(initial.size()), mSecret(secret), mOnAccept(std::move(onAccept)),
	  mTitleText(window, title, Font::get(FONT_SIZE_MEDIUM), TextColor, ALIGN_LEFT),
	  mFieldText(window, "", Font::get(FONT_SIZE_MEDIUM), TextColor, ALIGN_LEFT)
{
	setSize(static_cast<float>(Renderer::getScreenWidth()), static_cast<float>(Renderer::getScreenHeight()));
	for(int index = 0; index < LetterRows * Columns + SpecialCount; ++index)
		mKeyTexts.emplace_back(new TextComponent(window, "", Font::get(FONT_SIZE_MEDIUM), TextColor, ALIGN_CENTER));
	refreshLabels();
}

int GuiVirtualKeyboard::keysInRow(int row) const
{
	return row == LetterRows ? static_cast<int>(SpecialCount) : Columns;
}

int GuiVirtualKeyboard::specialStart(int key) const
{
	return SpecialLayouts[static_cast<std::size_t>(key)].start;
}

int GuiVirtualKeyboard::specialWidth(int key) const
{
	return SpecialLayouts[static_cast<std::size_t>(key)].width;
}

int GuiVirtualKeyboard::specialAt(int column) const
{
	for(int key = 0; key < SpecialCount; ++key)
		if(column >= specialStart(key) && column < specialStart(key) + specialWidth(key))
			return key;
	return SpecialCount - 1;
}

char GuiVirtualKeyboard::keyCharacter(int row, int column) const
{
	const bool upper = mShift == Shift::Once || (mShift == Shift::Caps && row > 0);
	const auto& rows = upper ? UpperRows : LowerRows;
	return rows[static_cast<std::size_t>(row)][column];
}

void GuiVirtualKeyboard::refreshLabels()
{
	for(int row = 0; row < LetterRows; ++row)
		for(int column = 0; column < Columns; ++column)
			mKeyTexts[static_cast<std::size_t>(row * Columns + column)]->setText(std::string(1, keyCharacter(row, column)));

	const std::array<std::string, SpecialCount> labels = {
		mShift == Shift::Off ? "SHIFT" : (mShift == Shift::Once ? "SHIFT ON" : "CAPS ON"), "<", ">", "SPACE", "DEL", "OK" };
	for(int key = 0; key < SpecialCount; ++key)
		mKeyTexts[static_cast<std::size_t>(LetterRows * Columns + key)]->setText(labels[static_cast<std::size_t>(key)]);

	std::string shown = mSecret && !mShowSecret ? std::string(mText.size(), '*') : mText;
	shown.insert(mCursor, "|");
	mFieldText.setText(shown);
	updateHelpPrompts();
}

void GuiVirtualKeyboard::moveSelection(int rowDelta, int columnDelta)
{
	if(rowDelta != 0)
	{
		const int x = mRow == LetterRows ? specialStart(mColumn) : mColumn;
		mRow = (mRow + rowDelta + Rows) % Rows;
		mColumn = mRow == LetterRows ? specialAt(x) : x;
	}
	if(columnDelta != 0)
	{
		const int count = keysInRow(mRow);
		mColumn = (mColumn + columnDelta + count) % count;
	}
}

void GuiVirtualKeyboard::insert(char character)
{
	if(mText.size() >= (mSecret ? MaxSecretLength : MaxTextLength))
		return;
	mText.insert(mCursor, 1, character);
	++mCursor;
}

void GuiVirtualKeyboard::erase()
{
	if(mCursor == 0)
		return;
	mText.erase(mCursor - 1, 1);
	--mCursor;
}

void GuiVirtualKeyboard::moveCursor(int delta)
{
	if(delta < 0 && mCursor > 0)
		--mCursor;
	else if(delta > 0 && mCursor < mText.size())
		++mCursor;
}

void GuiVirtualKeyboard::finish(bool accepted)
{
	const std::function<void(const std::string&)> callback = mOnAccept;
	const std::string text = mText;
	delete this;
	if(accepted && callback)
		callback(text);
}

void GuiVirtualKeyboard::pressKey()
{
	if(mRow < LetterRows)
	{
		insert(keyCharacter(mRow, mColumn));
		if(mShift == Shift::Once)
			mShift = Shift::Off;
		refreshLabels();
		return;
	}

	switch(mColumn)
	{
	case ShiftKey:
		mShift = mShift == Shift::Off ? Shift::Once : (mShift == Shift::Once ? Shift::Caps : Shift::Off);
		break;
	case CursorLeft:
		moveCursor(-1);
		break;
	case CursorRight:
		moveCursor(1);
		break;
	case Space:
		insert(' ');
		break;
	case Delete:
		erase();
		break;
	default:
		finish(true);
		return;
	}
	refreshLabels();
}

bool GuiVirtualKeyboard::input(InputConfig* config, Input input)
{
	if(input.value == 0)
		return true;

	if(config->isMappedLike("up", input))
		moveSelection(-1, 0);
	else if(config->isMappedLike("down", input))
		moveSelection(1, 0);
	else if(config->isMappedLike("left", input))
		moveSelection(0, -1);
	else if(config->isMappedLike("right", input))
		moveSelection(0, 1);
	else if(config->isMappedTo("a", input))
		pressKey();
	else if(config->isMappedTo("y", input))
	{
		erase();
		refreshLabels();
	}
	else if(config->isMappedTo("leftshoulder", input))
	{
		moveCursor(-1);
		refreshLabels();
	}
	else if(config->isMappedTo("rightshoulder", input))
	{
		moveCursor(1);
		refreshLabels();
	}
	else if(config->isMappedTo("select", input) && mSecret)
	{
		mShowSecret = !mShowSecret;
		refreshLabels();
	}
	else if(config->isMappedTo("b", input))
		finish(false);
	else if(config->isMappedTo("start", input))
		finish(true);

	return true;
}

void GuiVirtualKeyboard::render(const Transform4x4f& parentTrans)
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());
	const Transform4x4f trans = parentTrans * getTransform();
	const auto fill = [&trans](float x, float y, float w, float h, unsigned int color) {
		Renderer::setMatrix(trans);
		Renderer::drawRect(x, y, w, h, color, color);
	};

	const float panelX = width * PanelLeft;
	const float panelY = height * PanelTop;
	const float panelW = width * PanelWidth;
	const float panelH = height * PanelHeight;
	fill(panelX, panelY, panelW, panelH, PanelColor);

	const float margin = panelW * 0.04f;
	const float lineHeight = Font::get(FONT_SIZE_MEDIUM)->getHeight();
	mTitleText.setPosition(panelX + margin, panelY + margin * 0.6f);
	mTitleText.setSize(panelW - 2 * margin, lineHeight);
	mTitleText.render(trans);

	const float fieldY = panelY + margin * 0.6f + lineHeight * 1.4f;
	const float fieldH = lineHeight * 1.5f;
	fill(panelX + margin, fieldY, panelW - 2 * margin, fieldH, FieldColor);
	mFieldText.setPosition(panelX + margin * 1.4f, fieldY + (fieldH - lineHeight) / 2);
	mFieldText.setSize(panelW - 3 * margin, lineHeight);
	mFieldText.render(trans);

	const float gridY = fieldY + fieldH + margin * 0.6f;
	const float gridW = panelW - 2 * margin;
	const float gap = 6.0f;
	const float cellW = (gridW - gap * (Columns - 1)) / Columns;
	const float cellH = (panelY + panelH - margin * 0.6f - gridY - gap * (Rows - 1)) / Rows;

	for(int row = 0; row < Rows; ++row)
	{
		for(int column = 0; column < keysInRow(row); ++column)
		{
			const int start = row == LetterRows ? specialStart(column) : column;
			const int span = row == LetterRows ? specialWidth(column) : 1;
			const float x = panelX + margin + start * (cellW + gap);
			const float y = gridY + row * (cellH + gap);
			const float keyW = cellW * span + gap * (span - 1);
			const bool selected = row == mRow && column == mColumn;
			const bool active = row == LetterRows && column == ShiftKey && mShift != Shift::Off;
			fill(x, y, keyW, cellH, selected ? SelectedColor : (active ? ActiveKeyColor : KeyColor));

			TextComponent& label = *mKeyTexts[static_cast<std::size_t>(row * Columns + column)];
			label.setPosition(x, y + (cellH - lineHeight) / 2);
			label.setSize(keyW, lineHeight);
			label.setColor(selected ? SelectedTextColor : TextColor);
			label.render(trans);
		}
	}
}

std::vector<HelpPrompt> GuiVirtualKeyboard::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "type"));
	prompts.push_back(HelpPrompt("y", "delete"));
	prompts.push_back(HelpPrompt("l", "cursor"));
	prompts.push_back(HelpPrompt("r", "cursor"));
	if(mSecret)
		prompts.push_back(HelpPrompt("select", mShowSecret ? "hide" : "show"));
	prompts.push_back(HelpPrompt("start", "ok"));
	prompts.push_back(HelpPrompt("b", "cancel"));
	return prompts;
}
