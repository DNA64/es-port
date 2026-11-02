/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "guis/GuiToolRunner.h"

#include "guis/GuiMsgBox.h"
#include "renderers/Renderer.h"
#include "Window.h"
#include <chrono>
#include <utility>

GuiToolRunner::GuiToolRunner(Window* window, std::function<std::string()> work, std::function<void(const std::string&)> done)
	: GuiComponent(window), mBusy(window), mResult(std::async(std::launch::async, std::move(work))), mDone(std::move(done))
{
	setSize(static_cast<float>(Renderer::getScreenWidth()), static_cast<float>(Renderer::getScreenHeight()));
	mBusy.setSize(mSize);
}

bool GuiToolRunner::input(InputConfig*, Input)
{
	return true;
}

void GuiToolRunner::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
	mBusy.update(deltaTime);

	if(mResult.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		return;

	Window* window = mWindow;
	const std::string text = mResult.get();
	const std::function<void(const std::string&)> done = mDone;
	delete this;
	if(done)
		done(text);
	else
		window->pushGui(new GuiMsgBox(window, text, "OK", [] { return; }));
}

void GuiToolRunner::render(const Transform4x4f& parentTrans)
{
	const Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);
	Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0x00000090, 0x00000090);
	mBusy.render(trans);
}
