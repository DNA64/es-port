/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_GUIS_GUI_VIRTUAL_KEYBOARD_H
#define ES_APP_GUIS_GUI_VIRTUAL_KEYBOARD_H

#include "components/TextComponent.h"
#include "GuiComponent.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class GuiVirtualKeyboard : public GuiComponent
{
public:
	GuiVirtualKeyboard(Window* window, const std::string& title, const std::string& initial, bool secret,
		std::function<void(const std::string&)> onAccept);

	bool input(InputConfig* config, Input input) override;
	void render(const Transform4x4f& parentTrans) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	static constexpr int Columns = 10;
	static constexpr int Rows = 5;
	static constexpr int SpecialKeys = 5;

	enum Special { Shift, Space, Delete, Cancel, Accept };

	int keysInRow(int row) const;
	void pressKey();
	void moveSelection(int rowDelta, int columnDelta);
	void refreshLabels();
	void finish(bool accepted);

	std::string mText;
	bool mSecret;
	bool mShowSecret = false;
	bool mShifted = false;
	int mRow = 0;
	int mColumn = 0;
	std::function<void(const std::string&)> mOnAccept;

	TextComponent mTitleText;
	TextComponent mFieldText;
	std::vector<std::unique_ptr<TextComponent>> mKeyTexts;
};

#endif // ES_APP_GUIS_GUI_VIRTUAL_KEYBOARD_H
