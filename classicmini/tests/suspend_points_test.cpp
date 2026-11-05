/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "SuspendPoints.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
	int failures = 0;

	void check(bool condition, const char* what)
	{
		if(!condition)
		{
			std::cerr << "FAIL: " << what << "\n";
			++failures;
		}
	}

	void run(const std::string& command)
	{
		if(std::system(command.c_str()) != 0)
			std::cerr << "command failed: " << command << "\n";
	}

	bool isDirectory(const std::string& path)
	{
		struct stat info;
		return ::stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
	}

	void makePoint(const std::string& dir, const std::string& playtime)
	{
		run("mkdir -p '" + dir + "/rollback'");
		std::ofstream(dir + "/rollback/savestate") << "state of " << dir;
		std::ofstream(dir + "/state.png") << "png";
		std::ofstream(dir + "/state.time") << playtime;
	}
}

int main()
{
	char pattern[] = "/tmp/sp_test_XXXXXX";
	const std::string root = ::mkdtemp(pattern);
	const std::string profile = root + "/profile";
	const std::string vol = root + "/volatile";
	const SuspendPoints points(profile, vol);

	check(!points.floating().valid, "no floating at first");
	check(points.save("CLV-A", 1, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::NoFloating, "save without floating");

	makePoint(vol + "/CLV-A/suspendpoint0", "120");
	run("mkdir -p '" + vol + "/CLV-B/suspendpoint0'");
	const SuspendPoint floating = points.floating();
	check(floating.valid && floating.code == "CLV-A" && floating.playtime == 120, "floating found, empty one ignored");

	check(points.save("CLV-B", 1, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::WrongGame, "floating belongs to another game");
	check(points.save("CLV-A", 0, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::BadSlot, "slot 0 rejected");
	check(points.save("CLV-A", 5, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::BadSlot, "slot 5 rejected");
	check(points.save("CLV-A", 2, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::Saved, "save into empty slot");
	check(!points.floating().valid, "floating consumed");
	check(points.get("CLV-A", 2).valid && points.get("CLV-A", 2).playtime == 120, "slot 2 readable");
	check(!points.get("CLV-A", 1).valid, "slot 1 still empty");

	makePoint(vol + "/CLV-A/suspendpoint0", "300");
	check(points.save("CLV-A", 2, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::Occupied, "occupied without overwrite");
	check(points.lock("CLV-A", 2) && points.get("CLV-A", 2).locked, "lock slot");
	check(points.save("CLV-A", 2, SuspendPoints::SaveMode::Overwrite) == SuspendPoints::SaveResult::Locked, "locked slot refuses overwrite");
	check(points.unlock("CLV-A", 2) && !points.get("CLV-A", 2).locked, "unlock slot");
	check(points.save("CLV-A", 2, SuspendPoints::SaveMode::Overwrite) == SuspendPoints::SaveResult::Saved, "overwrite unlocked slot");
	check(points.get("CLV-A", 2).playtime == 300, "overwritten content");
	check(points.get("CLV-A", 2).thumbnail == profile + "/CLV-A/suspendpoint2/state.png", "thumbnail path");

	makePoint(vol + "/CLV-A/suspendpoint0", "500");
	check(points.save("CLV-A", 4, SuspendPoints::SaveMode::KeepExisting) == SuspendPoints::SaveResult::Saved, "save into slot 4");
	check(points.swap("CLV-A", 2, 4), "swap two slots");
	check(points.get("CLV-A", 2).playtime == 500 && points.get("CLV-A", 4).playtime == 300, "swapped contents");
	check(points.swap("CLV-A", 4, 3), "move into an empty slot");
	check(!points.get("CLV-A", 4).valid && points.get("CLV-A", 3).playtime == 300, "moved contents");

	const auto slots = points.slots("CLV-A");
	check(!slots[0].valid && slots[1].valid && slots[2].valid && !slots[3].valid, "slots() reflects slots 2 and 3");

	run("mkdir -p '" + profile + "/CLV-A/suspendpoint1'");
	check(!points.get("CLV-A", 1).valid, "directory without state.png/state.time is invalid");

	check(points.remove("CLV-A", 2) && !points.get("CLV-A", 2).valid, "remove slot");

	makePoint(profile + "/CLV-A/suspendpoint3.old", "777");
	run("rm -rf '" + profile + "/CLV-A/suspendpoint3'");
	check(points.get("CLV-A", 3).valid && points.get("CLV-A", 3).playtime == 777, "interrupted replacement restores the previous point");
	check(!isDirectory(profile + "/CLV-A/suspendpoint3.old"), "restored point no longer left as .old");

	makePoint(profile + "/CLV-A/suspendpoint3.old", "1");
	check(points.get("CLV-A", 3).playtime == 777, "a stale .old does not replace a present point");

	makePoint(vol + "/CLV-A/suspendpoint0", "900");
	check(points.save("CLV-A", 3, SuspendPoints::SaveMode::Overwrite) == SuspendPoints::SaveResult::Saved, "overwrite with a stale .old around");
	check(points.get("CLV-A", 3).playtime == 900, "overwritten content after a stale .old");
	check(!isDirectory(profile + "/CLV-A/suspendpoint3.old") && !isDirectory(profile + "/CLV-A/suspendpoint3.new"), "no .old or .new left after a save");

	makePoint(vol + "/CLV-A/suspendpoint0", "1");
	check(points.discardFloating() && !points.floating().valid, "discard floating");

	run("rm -rf '" + root + "'");
	if(failures == 0)
		std::cout << "all suspend point checks passed\n";
	return failures == 0 ? 0 : 1;
}
