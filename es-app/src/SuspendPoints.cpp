/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "SuspendPoints.h"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

constexpr int SuspendPoints::MaxSlots;

namespace
{
	constexpr const char* FloatingName = "suspendpoint0";
	constexpr const char* LockedName = "es.locked";
	constexpr const char* StagingSuffix = ".new";
	constexpr const char* SwapSuffix = ".swap";
	constexpr mode_t DirectoryMode = 0755;

	using DirHandle = std::unique_ptr<DIR, int (*)(DIR*)>;

	bool readStat(const std::string& path, struct stat& info) noexcept
	{
		return ::stat(path.c_str(), &info) == 0;
	}

	bool isDirectory(const std::string& path) noexcept
	{
		struct stat info;
		return readStat(path, info) && S_ISDIR(info.st_mode);
	}

	bool isFile(const std::string& path) noexcept
	{
		struct stat info;
		return readStat(path, info) && S_ISREG(info.st_mode);
	}

	long modifiedTime(const std::string& path) noexcept
	{
		struct stat info;
		return readStat(path, info) ? static_cast<long>(info.st_mtime) : 0;
	}

	std::string environmentOr(const char* name, const char* fallback)
	{
		const char* value = std::getenv(name);
		return value != nullptr ? value : fallback;
	}

	std::vector<std::string> listDirectory(const std::string& path)
	{
		std::vector<std::string> names;
		const DirHandle dir(::opendir(path.c_str()), &::closedir);
		if(!dir)
			return names;

		while(const dirent* entry = ::readdir(dir.get()))
		{
			const std::string name = entry->d_name;
			if(name != "." && name != "..")
				names.push_back(name);
		}
		return names;
	}

	bool makeDirectories(const std::string& path)
	{
		if(isDirectory(path))
			return true;

		const std::string::size_type slash = path.find_last_of('/');
		if(slash != std::string::npos && slash > 0 && !makeDirectories(path.substr(0, slash)))
			return false;

		return ::mkdir(path.c_str(), DirectoryMode) == 0 || errno == EEXIST;
	}

	bool removeTree(const std::string& path)
	{
		if(isDirectory(path))
		{
			const std::vector<std::string> names = listDirectory(path);
			const bool emptied = std::all_of(names.begin(), names.end(),
				[&path](const std::string& name) { return removeTree(path + "/" + name); });
			return emptied && ::rmdir(path.c_str()) == 0;
		}
		return ::unlink(path.c_str()) == 0 || errno == ENOENT;
	}

	bool copyFile(const std::string& from, const std::string& to)
	{
		std::ifstream in(from, std::ios::binary);
		std::ofstream out(to, std::ios::binary | std::ios::trunc);
		if(!in || !out)
			return false;

		out << in.rdbuf();
		out.flush();
		return static_cast<bool>(out);
	}

	bool copyTree(const std::string& from, const std::string& to)
	{
		if(!isDirectory(from))
			return copyFile(from, to);

		if(!makeDirectories(to))
			return false;

		const std::vector<std::string> names = listDirectory(from);
		return std::all_of(names.begin(), names.end(),
			[&from, &to](const std::string& name) { return copyTree(from + "/" + name, to + "/" + name); });
	}

	bool moveTree(const std::string& from, const std::string& to)
	{
		if(::rename(from.c_str(), to.c_str()) == 0)
			return true;
		if(errno != EXDEV)
			return false;

		if(!copyTree(from, to))
		{
			static_cast<void>(removeTree(to));
			return false;
		}
		return removeTree(from);
	}

	bool isValidPoint(const std::string& dir)
	{
		return (isFile(dir + "/state") || isDirectory(dir + "/rollback"))
			&& isFile(dir + "/state.png")
			&& isFile(dir + "/state.time");
	}

	long readPlaytime(const std::string& dir)
	{
		std::ifstream in(dir + "/state.time");
		long value = 0;
		in >> value;
		return in ? value : 0;
	}

	SuspendPoint describe(const std::string& code, int slot, const std::string& dir)
	{
		SuspendPoint point;
		if(!isValidPoint(dir))
			return point;

		point.valid = true;
		point.slot = slot;
		point.code = code;
		point.dir = dir;
		point.thumbnail = dir + "/state.png";
		point.playtime = readPlaytime(dir);
		point.locked = isFile(dir + "/" + LockedName);
		return point;
	}

	bool validSlot(int slot) noexcept
	{
		return slot >= 1 && slot <= SuspendPoints::MaxSlots;
	}
}

SuspendPoints::SuspendPoints(std::string profileDir, std::string volatileDir)
	: mProfileDir(std::move(profileDir)), mVolatileDir(std::move(volatileDir))
{
}

SuspendPoints SuspendPoints::fromEnvironment()
{
	return SuspendPoints(environmentOr("ES_SP_PROFILE", "/var/lib/clover/profiles/0"),
		environmentOr("ES_SP_VOLATILE", "/var/cache/clover/volatile"));
}

std::string SuspendPoints::slotDir(const std::string& code, int slot) const
{
	if(slot == 0)
		return mVolatileDir + "/" + code + "/" + FloatingName;
	return mProfileDir + "/" + code + "/suspendpoint" + std::to_string(slot);
}

SuspendPoint SuspendPoints::floating() const
{
	SuspendPoint newest;
	long newestTime = 0;

	for(const std::string& code : listDirectory(mVolatileDir))
	{
		SuspendPoint candidate = describe(code, 0, slotDir(code, 0));
		if(!candidate.valid)
			continue;

		const long time = modifiedTime(candidate.dir + "/state.time");
		if(!newest.valid || time > newestTime)
		{
			newestTime = time;
			newest = std::move(candidate);
		}
	}
	return newest;
}

SuspendPoint SuspendPoints::get(const std::string& code, int slot) const
{
	if(!validSlot(slot))
		return SuspendPoint();
	return describe(code, slot, slotDir(code, slot));
}

std::array<SuspendPoint, SuspendPoints::MaxSlots> SuspendPoints::slots(const std::string& code) const
{
	std::array<SuspendPoint, MaxSlots> result;
	for(std::size_t index = 0; index < result.size(); ++index)
		result[index] = get(code, static_cast<int>(index) + 1);
	return result;
}

SuspendPoints::SaveResult SuspendPoints::save(const std::string& code, int slot, SaveMode mode) const
{
	if(!validSlot(slot))
		return SaveResult::BadSlot;

	const SuspendPoint pending = describe(code, 0, slotDir(code, 0));
	if(!pending.valid)
		return floating().valid ? SaveResult::WrongGame : SaveResult::NoFloating;

	const SuspendPoint current = get(code, slot);
	if(current.valid)
	{
		if(current.locked)
			return SaveResult::Locked;
		if(mode == SaveMode::KeepExisting)
			return SaveResult::Occupied;
	}

	const std::string target = slotDir(code, slot);
	const std::string staging = target + StagingSuffix;
	if(!makeDirectories(mProfileDir + "/" + code) || !removeTree(staging) || !moveTree(pending.dir, staging))
		return SaveResult::Failed;

	if(!removeTree(target) || ::rename(staging.c_str(), target.c_str()) != 0)
	{
		static_cast<void>(removeTree(staging));
		return SaveResult::Failed;
	}
	return SaveResult::Saved;
}

bool SuspendPoints::remove(const std::string& code, int slot) const
{
	return validSlot(slot) && removeTree(slotDir(code, slot));
}

bool SuspendPoints::lock(const std::string& code, int slot) const
{
	const SuspendPoint point = get(code, slot);
	if(!point.valid)
		return false;

	const std::ofstream marker(point.dir + "/" + LockedName);
	return static_cast<bool>(marker);
}

bool SuspendPoints::unlock(const std::string& code, int slot) const
{
	const SuspendPoint point = get(code, slot);
	if(!point.valid)
		return false;

	return ::unlink((point.dir + "/" + LockedName).c_str()) == 0 || errno == ENOENT;
}

bool SuspendPoints::swap(const std::string& code, int first, int second) const
{
	if(first == second || !validSlot(first) || !validSlot(second))
		return false;

	const std::string a = slotDir(code, first);
	const std::string b = slotDir(code, second);
	const std::string spare = a + SwapSuffix;

	const bool hasFirst = isDirectory(a);
	const bool hasSecond = isDirectory(b);
	if(!hasFirst && !hasSecond)
		return false;

	if(hasFirst && ::rename(a.c_str(), spare.c_str()) != 0)
		return false;

	if(hasSecond && ::rename(b.c_str(), a.c_str()) != 0)
	{
		if(hasFirst)
			static_cast<void>(::rename(spare.c_str(), a.c_str()));
		return false;
	}
	return !hasFirst || ::rename(spare.c_str(), b.c_str()) == 0;
}

bool SuspendPoints::discardFloating() const
{
	const SuspendPoint pending = floating();
	return !pending.valid || removeTree(pending.dir);
}
