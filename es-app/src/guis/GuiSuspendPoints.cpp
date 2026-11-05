/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiSuspendPoints.h"

#include "MenuColors.h"

#include "guis/GuiMsgBox.h"
#include "UiSound.h"
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
	constexpr unsigned int GaugeColor = 0xF0F0F0FF;
	constexpr unsigned int TextColor = 0xF0F0F0FF;
	constexpr unsigned int MutedColor = 0x9A9A9AFF;

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

SuspendBox::SuspendBox(Window* window, const std::string& defaultPath, int corner)
	: mPatch(window, defaultPath), mImage(window)
{
	mPatch.setCornerSize(corner, corner);
}

void SuspendBox::load(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element)
{
	const ThemeData::ThemeElement* image = theme->getElement(view, element, "image");
	mFullImage = image != nullptr && image->has("path");
	if(mFullImage)
		mImage.setImage(image->get<std::string>("path"));
	else
		mPatch.applyTheme(theme, view, element, ThemeFlags::PATH);
}

void SuspendBox::setSize(float width, float height)
{
	mPatch.setSize(width, height);
	mImage.setResize(width, height);
}

void SuspendBox::render(const Transform4x4f& trans)
{
	if(mFullImage)
		mImage.render(trans);
	else
		mPatch.render(trans);
}

GuiSuspendPoints::GuiSuspendPoints(Window* window, std::string title, std::string code, SuspendPoints points, Pad pad,
	std::function<void(int)> onResume, std::function<void()> onChanged)
	: GuiComponent(window), mTitle(std::move(title)), mCode(std::move(code)), mPoints(std::move(points)), mPad(pad),
	  mOnResume(std::move(onResume)), mOnChanged(std::move(onChanged)),
	  mPanel(window, ":/suspend_card.png", CardCorner), mCell(window, ":/suspend_card.png", CardCorner),
	  mCellSelected(window, ":/suspend_card_sel.png", CardCorner),
	  mFrame(window, ":/suspend_frame.png", CardCorner), mFrameSelected(window, ":/suspend_frame_sel.png", CardCorner),
	  mHeading(window, "", Font::get(FONT_SIZE_MEDIUM), TextColor),
	  mNotice(window, "", Font::get(FONT_SIZE_SMALL), MutedColor, ALIGN_RIGHT),
	  mLock(window)
{
	mLock.setImage(":/suspend_lock.png");
	for(int slot = 0; slot < SuspendPoints::MaxSlots; ++slot)
	{
		mShots.emplace_back(new ImageComponent(window));
		mLabels.emplace_back(new TextComponent(window, "", Font::get(FONT_SIZE_SMALL), TextColor, ALIGN_CENTER));
	}

	layout();
	refresh();
}

void GuiSuspendPoints::loadTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view)
{
	constexpr unsigned int textProperties = ThemeFlags::COLOR | ThemeFlags::FONT_PATH | ThemeFlags::FONT_SIZE;
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());

	const auto rectOf = [&](const char* name, const char* type) {
		const ThemeData::ThemeElement* elem = theme->getElement(view, name, type);
		if(elem == nullptr || !elem->has("pos") || !elem->has("size"))
			return Rect();
		const Vector2f pos = elem->get<Vector2f>("pos");
		const Vector2f size = elem->get<Vector2f>("size");
		return Rect(pos.x() * width, pos.y() * height, size.x() * width, size.y() * height, true);
	};
	const auto boxRect = [&](const char* name) -> Rect {
		const ThemeData::ThemeElement* elem = theme->getElement(view, name, "");
		return elem != nullptr && elem->type == "image" ? rectOf(name, "image") : rectOf(name, "ninepatch");
	};

	mPanel.load(theme, view, "suspend_panel");
	mCell.load(theme, view, "suspend_cell");
	mCellSelected.load(theme, view, "suspend_cell_selected");
	mFrame.load(theme, view, "suspend_frame");
	mFrameSelected.load(theme, view, "suspend_frame_selected");
	mPanelTheme = boxRect("suspend_panel");
	mCellTheme = boxRect("suspend_cell");
	mShotTheme = rectOf("suspend_shot", "image");
	mLabelTheme = rectOf("suspend_label", "text");
	mFrameTheme = boxRect("suspend_frame");
	mFrameSelectedTheme = boxRect("suspend_frame_selected");
	mLockTheme = rectOf("suspend_lock", "image");
	mGaugeTheme = rectOf("suspend_gauge", "image");
	mNextTheme = rectOf("suspend_cell_next", "image");
	mGaugeColor = GaugeColor;
	if(const ThemeData::ThemeElement* gauge = theme->getElement(view, "suspend_gauge", "image"))
		if(gauge->has("color"))
			mGaugeColor = gauge->get<unsigned int>("color");

	layout();

	mHeading.applyTheme(theme, view, "suspend_title", textProperties | ThemeFlags::POSITION | ThemeFlags::SIZE);
	mNotice.applyTheme(theme, view, "suspend_notice", textProperties | ThemeFlags::POSITION | ThemeFlags::SIZE);
	for(auto& label : mLabels)
		label->applyTheme(theme, view, "suspend_label", textProperties);
}

void GuiSuspendPoints::layout()
{
	const float width = static_cast<float>(Renderer::getScreenWidth());
	const float height = static_cast<float>(Renderer::getScreenHeight());
	const float count = static_cast<float>(mShots.size());

	setSize(width, height);
	mHeading.setPosition(width * (PanelLeft + 0.025f), height * (PanelTop + 0.015f));
	mHeading.setSize(width * 0.6f, height * 0.06f);
	mNotice.setPosition(width * (PanelLeft + PanelWidth - 0.325f), height * (PanelTop + 0.015f));
	mNotice.setSize(width * 0.3f, height * 0.06f);

	mPanelRect = mPanelTheme.set ? mPanelTheme : Rect(width * PanelLeft - Bleed, height * PanelTop - Bleed,
		width * PanelWidth + 2 * Bleed, height * PanelHeight + 2 * Bleed);
	mCellRect = mCellTheme.set ? mCellTheme : Rect(width * CellsLeft - Bleed, height * CellsTop - Bleed,
		width * CellWidth + 2 * Bleed, height * CellHeight + 2 * Bleed);
	if(mNextTheme.set)
		mCellStep = mNextTheme.x - mCellRect.x;
	else if(mCellTheme.set)
	{
		const float margin = mCellRect.x - mPanelRect.x;
		const float lastLeft = mPanelRect.x + mPanelRect.w - margin - mCellRect.w;
		mCellStep = count > 1 ? (lastLeft - mCellRect.x) / (count - 1) : 0.0f;
	}
	else
		mCellStep = width * (CellWidth + CellGap);

	const Rect shot = mShotTheme.set ? mShotTheme : Rect(width * CellsLeft + Border, height * CellsTop + Border,
		width * CellWidth - 2 * Border, height * ShotHeight);
	const Rect label = mLabelTheme.set ? mLabelTheme : Rect(width * CellsLeft, height * (CellsTop + ShotHeight + 0.01f),
		width * CellWidth, height * 0.07f);

	mFrameRect = mFrameTheme.set ? mFrameTheme : mCellRect;
	mFrameSelectedRect = mFrameSelectedTheme.set ? mFrameSelectedTheme : mFrameRect;
	mLockRect = mLockTheme.set ? mLockTheme : Rect(mCellRect.x + mCellRect.w - Bleed - height * 0.06f,
		mCellRect.y + Bleed + height * 0.01f, height * 0.05f, height * 0.05f);
	mGaugeRect = mGaugeTheme.set ? mGaugeTheme : Rect(mCellRect.x + Bleed,
		mCellRect.y + mCellRect.h - Bleed - Border * 2, mCellRect.w - 2 * Bleed, Border * 2);
	mLock.setResize(mLockRect.w, mLockRect.h);

	mPanel.setSize(mPanelRect.w, mPanelRect.h);
	mCell.setSize(mCellRect.w, mCellRect.h);
	mCellSelected.setSize(mCellRect.w, mCellRect.h);
	mFrame.setSize(mFrameRect.w, mFrameRect.h);
	mFrameSelected.setSize(mFrameSelectedRect.w, mFrameSelectedRect.h);

	for(std::size_t index = 0; index < mShots.size(); ++index)
	{
		const float shift = static_cast<float>(index) * mCellStep;
		mShots[index]->setPosition(shot.x + shift, shot.y);
		mShots[index]->setResize(shot.w, shot.h);
		mLabels[index]->setPosition(label.x + shift, label.y);
		mLabels[index]->setSize(label.w, label.h);
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
		UiSound::play("se_sys_push.wav");
		refresh();
		notify("SAVED");
		if(mOnChanged)
			mOnChanged();
		break;
	case SuspendPoints::SaveResult::Locked:
		UiSound::play("se_sys_attention.wav");
		notify("SLOT IS LOCKED");
		break;
	case SuspendPoints::SaveResult::Occupied:
		UiSound::play("se_sys_attention.wav");
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
	{
		mSelected = std::max(1, mSelected - 1);
		UiSound::play("se_sys_cursor.wav");
	}
	else if(config->isMappedLike("right", input))
	{
		mSelected = std::min(SuspendPoints::MaxSlots, mSelected + 1);
		UiSound::play("se_sys_cursor.wav");
	}
	else if(config->isMappedTo("start", input) || (mPad == Pad::Snes && config->isMappedTo("a", input)))
	{
		if(selectedPoint().valid)
		{
			const std::function<void(int)> resume = mOnResume;
			const int slot = mSelected;
			UiSound::play("se_sys_click_game.wav");
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
			UiSound::play(point.locked ? "se_sys_unlock.wav" : "se_sys_lock.wav");
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
				UiSound::play("se_sys_delete.wav");
				static_cast<void>(mPoints.remove(mCode, mSelected));
				refresh();
			}, "NO", nullptr));
		}
	}
	else if(config->isMappedLike("up", input) || config->isMappedTo("b", input))
	{
		UiSound::play("se_sys_cancel.wav");
		close();
	}

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
	panelTrans.translate(Vector3f(mPanelRect.x, mPanelRect.y, 0.0f));
	mPanel.render(panelTrans);

	mHeading.render(trans);
	mNotice.render(trans);

	for(std::size_t index = 0; index < mSlots.size(); ++index)
	{
		const float shift = static_cast<float>(index) * mCellStep;
		const bool selected = static_cast<int>(index) + 1 == mSelected;

		Transform4x4f cellTrans = trans;
		cellTrans.translate(Vector3f(mCellRect.x + shift, mCellRect.y, 0.0f));
		(selected ? mCellSelected : mCell).render(cellTrans);

		if(mSlots[index].valid)
			mShots[index]->render(trans);

		const Rect& frame = selected ? mFrameSelectedRect : mFrameRect;
		Transform4x4f frameTrans = trans;
		frameTrans.translate(Vector3f(frame.x + shift, frame.y, 0.0f));
		(selected ? mFrameSelected : mFrame).render(frameTrans);
		mLabels[index]->render(trans);

		if(mSlots[index].locked)
		{
			Transform4x4f lockTrans = trans;
			lockTrans.translate(Vector3f(mLockRect.x + shift, mLockRect.y, 0.0f));
			mLock.render(lockTrans);
		}

		if(selected && mHolding)
		{
			Renderer::setMatrix(trans);
			const float filled = mGaugeRect.w * static_cast<float>(mHeldMs) / static_cast<float>(HoldToOverwriteMs);
			Renderer::drawRect(mGaugeRect.x + shift, mGaugeRect.y, filled, mGaugeRect.h, mGaugeColor, mGaugeColor);
		}
	}
}

std::vector<HelpPrompt> GuiSuspendPoints::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", "slot"));
	prompts.push_back(HelpPrompt(mPad == Pad::Snes ? "a" : "start", "resume"));
	if(mFloating.valid)
		prompts.push_back(HelpPrompt(mPad == Pad::Snes ? "y" : "a", "save (hold to overwrite)"));
	prompts.push_back(HelpPrompt("down", "lock"));
	prompts.push_back(HelpPrompt("select", "delete"));
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
