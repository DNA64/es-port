/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiSuspendPoints.h"

#include "guis/GuiMsgBox.h"
#include "InputConfig.h"
#include "Window.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <utility>

namespace
{
	constexpr int HoldToOverwriteMs = 400;
	constexpr int NoticeMs = 1500;
	constexpr unsigned int GaugeColor = 0xF4F1FFFF;
	constexpr unsigned int TextColor = 0xF4F1FFFF;
	constexpr unsigned int MutedColor = 0xB9A8FFFF;

	constexpr float PanelLeft = 0.04f;
	constexpr float PanelTop = 0.46f;
	constexpr float PanelWidth = 0.92f;
	constexpr float PanelHeight = 0.40f;
	constexpr float CellsLeft = 0.07f;
	constexpr float CellsTop = 0.56f;
	constexpr float CellWidth = 0.19f;
	constexpr float CellGap = 0.0233f;
	constexpr float CellHeight = 0.27f;
	constexpr float ShotHeight = 0.17f;
	constexpr float Border = 4.0f;
	constexpr int CardCorner = 30;
	constexpr float Bleed = 12.0f;

	std::string clock(long seconds)
	{
		char text[32];
		std::snprintf(text, sizeof text, "%ld:%02ld", seconds / 60, seconds % 60);
		return text;
	}
}

GuiSuspendPoints::GuiSuspendPoints(Window* window, std::string title, std::string code, SuspendPoints points, Pad pad,
	std::function<void(int)> onResume, std::function<void()> onChanged)
	: GuiComponent(window), mTitle(std::move(title)), mCode(std::move(code)), mPoints(std::move(points)), mPad(pad),
	  mOnResume(std::move(onResume)), mOnChanged(std::move(onChanged)),
	  mPanel(window, ":/suspend_card.png"), mCell(window, ":/suspend_card.png"),
	  mCellSelected(window, ":/suspend_card_sel.png"),
	  mHeading(window, "", Font::get(FONT_SIZE_MEDIUM), TextColor),
	  mNotice(window, "", Font::get(FONT_SIZE_SMALL), MutedColor, ALIGN_RIGHT),
	  mLock(window)
{
	mLock.setImage(":/suspend_lock.png");
	mPanel.setCornerSize(CardCorner, CardCorner);
	mCell.setCornerSize(CardCorner, CardCorner);
	mCellSelected.setCornerSize(CardCorner, CardCorner);
	for(int slot = 0; slot < SuspendPoints::MaxSlots; ++slot)
	{
		mShots.emplace_back(new ImageComponent(window));
		mLabels.emplace_back(new TextComponent(window, "", Font::get(FONT_SIZE_SMALL), TextColor, ALIGN_CENTER));
	}

	layout();
	refresh();
}

void GuiSuspendPoints::layout()
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());

	setSize(width, height);
	mHeading.setPosition(width * (PanelLeft + 0.025f), height * (PanelTop + 0.015f));
	mHeading.setSize(width * 0.6f, height * 0.06f);
	mNotice.setPosition(width * (PanelLeft + PanelWidth - 0.325f), height * (PanelTop + 0.015f));
	mNotice.setSize(width * 0.3f, height * 0.06f);
	mLock.setResize(height * 0.05f, height * 0.05f);
	mPanel.setSize(width * PanelWidth + 2 * Bleed, height * PanelHeight + 2 * Bleed);
	mCell.setSize(width * CellWidth + 2 * Bleed, height * CellHeight + 2 * Bleed);
	mCellSelected.setSize(width * CellWidth + 2 * Bleed, height * CellHeight + 2 * Bleed);

	for(std::size_t index = 0; index < mShots.size(); ++index)
	{
		const float left = width * (CellsLeft + static_cast<float>(index) * (CellWidth + CellGap));
		mShots[index]->setPosition(left + Border, height * CellsTop + Border);
		mShots[index]->setResize(width * CellWidth - 2 * Border, height * ShotHeight);
		mLabels[index]->setPosition(left, height * (CellsTop + ShotHeight + 0.01f));
		mLabels[index]->setSize(width * CellWidth, height * 0.07f);
	}
}

const SuspendPoint& GuiSuspendPoints::selectedPoint() const
{
	return mSlots[static_cast<std::size_t>(mSelected - 1)];
}

void GuiSuspendPoints::refresh()
{
	mFloating = mPoints.floating();
	if(mFloating.valid && mFloating.code != mCode)
		mFloating = SuspendPoint();

	mSlots = mPoints.slots(mCode);
	mHeading.setText("SUSPEND POINT LIST  -  " + mTitle);

	for(std::size_t index = 0; index < mSlots.size(); ++index)
	{
		const SuspendPoint& point = mSlots[index];
		if(point.valid)
			mShots[index]->setImage(point.thumbnail);
		const std::string state = point.valid ? clock(point.playtime) : "EMPTY";
		mLabels[index]->setText(std::to_string(index + 1) + "   " + state);
	}
	updateHelpPrompts();
}

void GuiSuspendPoints::notify(const std::string& text)
{
	mNotice.setText(text);
	mNoticeMs = NoticeMs;
}

void GuiSuspendPoints::close()
{
	delete this;
}

bool GuiSuspendPoints::isSaveKey(InputConfig* config, const Input& input) const
{
	return config->isMappedTo(mPad == Pad::Snes ? "y" : "a", input);
}

void GuiSuspendPoints::saveSelected(SuspendPoints::SaveMode mode)
{
	switch(mPoints.save(mCode, mSelected, mode))
	{
	case SuspendPoints::SaveResult::Saved:
		refresh();
		notify("SAVED");
		if(mOnChanged)
			mOnChanged();
		break;
	case SuspendPoints::SaveResult::Locked:
		notify("SLOT IS LOCKED");
		break;
	case SuspendPoints::SaveResult::Occupied:
		notify("HOLD TO OVERWRITE");
		break;
	default:
		notify("NOTHING TO SAVE");
		break;
	}
}

bool GuiSuspendPoints::input(InputConfig* config, Input input)
{
	if(isSaveKey(config, input))
	{
		if(input.value == 0)
		{
			mHolding = false;
			mHeldMs = 0;
			return true;
		}

		const SuspendPoint& point = selectedPoint();
		if(!mFloating.valid)
			notify("NOTHING TO SAVE");
		else if(!point.valid)
			saveSelected(SuspendPoints::SaveMode::KeepExisting);
		else if(point.locked)
			notify("SLOT IS LOCKED");
		else
		{
			mHolding = true;
			mHeldMs = 0;
		}
		return true;
	}

	if(input.value == 0)
		return true;

	if(config->isMappedLike("left", input))
		mSelected = std::max(1, mSelected - 1);
	else if(config->isMappedLike("right", input))
		mSelected = std::min(SuspendPoints::MaxSlots, mSelected + 1);
	else if(config->isMappedTo("start", input))
	{
		if(selectedPoint().valid)
		{
			const std::function<void(int)> resume = mOnResume;
			const int slot = mSelected;
			close();
			if(resume)
				resume(slot);
		}
	}
	else if(config->isMappedLike("down", input))
	{
		const SuspendPoint& point = selectedPoint();
		if(point.valid)
		{
			static_cast<void>(point.locked ? mPoints.unlock(mCode, mSelected) : mPoints.lock(mCode, mSelected));
			refresh();
		}
	}
	else if(config->isMappedTo("select", input))
	{
		const SuspendPoint& point = selectedPoint();
		if(point.locked)
			notify("SLOT IS LOCKED");
		else if(point.valid)
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, "DELETE THIS SUSPEND POINT?", "YES", [this] {
				static_cast<void>(mPoints.remove(mCode, mSelected));
				refresh();
			}, "NO", nullptr));
		}
	}
	else if(config->isMappedLike("up", input) || config->isMappedTo("b", input))
		close();

	return true;
}

void GuiSuspendPoints::update(int deltaTime)
{
	if(mNoticeMs > 0)
	{
		mNoticeMs -= deltaTime;
		if(mNoticeMs <= 0)
			mNotice.setText("");
	}

	if(mHolding)
	{
		mHeldMs += deltaTime;
		if(mHeldMs >= HoldToOverwriteMs)
		{
			mHolding = false;
			mHeldMs = 0;
			saveSelected(SuspendPoints::SaveMode::Overwrite);
		}
	}
}

void GuiSuspendPoints::render(const Transform4x4f& parentTrans)
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());

	const Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);
	Transform4x4f panelTrans = trans;
	panelTrans.translate(Vector3f(width * PanelLeft - Bleed, height * PanelTop - Bleed, 0.0f));
	mPanel.render(panelTrans);

	mHeading.render(trans);
	mNotice.render(trans);

	for(std::size_t index = 0; index < mSlots.size(); ++index)
	{
		const float left = width * (CellsLeft + static_cast<float>(index) * (CellWidth + CellGap));
		const bool selected = static_cast<int>(index) + 1 == mSelected;

		Transform4x4f cellTrans = trans;
		cellTrans.translate(Vector3f(left - Bleed, height * CellsTop - Bleed, 0.0f));
		(selected ? mCellSelected : mCell).render(cellTrans);

		if(mSlots[index].valid)
			mShots[index]->render(trans);
		mLabels[index]->render(trans);

		if(mSlots[index].locked)
		{
			Transform4x4f lockTrans = trans;
			lockTrans.translate(Vector3f(left + width * CellWidth - height * 0.06f, height * CellsTop + height * 0.01f, 0.0f));
			mLock.render(lockTrans);
		}

		if(selected && mHolding)
		{
			Renderer::setMatrix(trans);
			const float filled = width * CellWidth * static_cast<float>(mHeldMs) / static_cast<float>(HoldToOverwriteMs);
			Renderer::drawRect(left, height * (CellsTop + CellHeight) - Border * 2, filled, Border * 2, GaugeColor, GaugeColor);
		}
	}
}

std::vector<HelpPrompt> GuiSuspendPoints::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", "slot"));
	prompts.push_back(HelpPrompt("start", "resume"));
	if(mFloating.valid)
		prompts.push_back(HelpPrompt(mPad == Pad::Snes ? "y" : "a", "save (hold to overwrite)"));
	prompts.push_back(HelpPrompt("down", "lock"));
	prompts.push_back(HelpPrompt("select", "delete"));
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
