/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_APP_SUSPEND_POINTS_H
#define ES_APP_SUSPEND_POINTS_H

#include <array>
#include <string>

struct SuspendPoint
{
	bool valid = false;
	bool locked = false;
	int slot = 0;
	long playtime = 0;
	std::string code;
	std::string dir;
	std::string thumbnail;
};

class SuspendPoints
{
public:
	static constexpr int MaxSlots = 4;

	enum class SaveMode { KeepExisting, Overwrite };
	enum class SaveResult { Saved, NoFloating, WrongGame, BadSlot, Occupied, Locked, Failed };

	SuspendPoints(std::string profileDir, std::string volatileDir);

	static SuspendPoints fromEnvironment();

	SuspendPoint floating() const;
	SuspendPoint get(const std::string& code, int slot) const;
	std::array<SuspendPoint, MaxSlots> slots(const std::string& code) const;

	SaveResult save(const std::string& code, int slot, SaveMode mode) const;
	bool remove(const std::string& code, int slot) const;
	bool lock(const std::string& code, int slot) const;
	bool unlock(const std::string& code, int slot) const;
	bool swap(const std::string& code, int first, int second) const;
	bool discardFloating() const;

private:
	std::string slotDir(const std::string& code, int slot) const;

	std::string mProfileDir;
	std::string mVolatileDir;
};

#endif // ES_APP_SUSPEND_POINTS_H
