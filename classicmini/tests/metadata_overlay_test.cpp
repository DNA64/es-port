/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "MetaData.h"

#include <iostream>
#include <pugixml.hpp>
#include <string>

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

	pugi::xml_document parse(const char* xml)
	{
		pugi::xml_document doc;
		doc.load_string(xml);
		return doc;
	}
}

int main()
{
	const std::string root = "/tmp/es-library/snes";

	auto baseDoc = parse("<game><name>Foo</name><desc>hakchi desc</desc><developer>CLV-U-AAAAA</developer>"
		"<genre>1993 - 1 Player</genre><players>2</players></game>");
	pugi::xml_node baseNode = baseDoc.child("game");
	const MetaDataList base = MetaDataList::createFromXML(GAME_METADATA, baseNode, root);

	MetaDataList current = base;
	auto overlayDoc = parse("<game><path>./x.desktop</path><desc>scraped desc</desc><developer>Quintet</developer>"
		"<rating>0.8</rating><image>/media/data/emulationstation/downloaded_images/snes/x-image.png</image></game>");
	pugi::xml_node overlayNode = overlayDoc.child("game");
	current.mergeFromXML(overlayNode, root);

	check(current.get("desc") == "scraped desc", "overlay overrides desc");
	check(current.get("developer") == "Quintet", "overlay overrides developer");
	check(current.get("name") == "Foo", "fields missing from the overlay keep the base value");
	check(current.get("genre") == "1993 - 1 Player", "genre untouched");
	check(current.get("players") == "2", "players untouched");
	check(current.get("rating") == "0.8", "overlay adds rating");
	check(current.get("image") == "/media/data/emulationstation/downloaded_images/snes/x-image.png", "overlay image path kept");

	pugi::xml_document out;
	pugi::xml_node node = out.append_child("game");
	current.appendDiffToXML(node, base, root);
	check(!node.child("name"), "unchanged name is not written");
	check(!node.child("genre"), "unchanged genre is not written");
	check(!node.child("players"), "unchanged players is not written");
	check(std::string(node.child("desc").text().get()) == "scraped desc", "changed desc is written");
	check(std::string(node.child("developer").text().get()) == "Quintet", "changed developer is written");
	check(std::string(node.child("rating").text().get()) == "0.8", "changed rating is written");

	MetaDataList replay = base;
	replay.mergeFromXML(node, root);
	check(replay.get("desc") == current.get("desc") && replay.get("image") == current.get("image"), "written diff replays to the same values");

	MetaDataList later = base;
	auto renamedDoc = parse("<game><name>Bar</name><desc>hakchi desc 2</desc><developer>CLV-U-AAAAA</developer>"
		"<genre>1993 - 1 Player</genre><players>2</players></game>");
	pugi::xml_node renamedNode = renamedDoc.child("game");
	const MetaDataList newBase = MetaDataList::createFromXML(GAME_METADATA, renamedNode, root);
	MetaDataList merged = newBase;
	merged.mergeFromXML(node, root);
	check(merged.get("name") == "Bar", "a hakchi rename still shows through the overlay");
	check(merged.get("desc") == "scraped desc", "the scraped desc still wins after a hakchi resync");

	pugi::xml_document none;
	pugi::xml_node emptyNode = none.append_child("game");
	base.appendDiffToXML(emptyNode, base, root);
	check(!emptyNode.first_child(), "identical metadata writes nothing");

	if(failures == 0)
		std::cout << "metadata overlay tests passed\n";
	return failures == 0 ? 0 : 1;
}
