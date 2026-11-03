/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#pragma once
#ifndef ES_CORE_MUSIC_PLAYER_H
#define ES_CORE_MUSIC_PLAYER_H

#include <SDL_audio.h>
#include <memory>
#include <string>
#include <vector>

class MusicTrack;

class MusicPlayer
{
public:
	static MusicPlayer& get();

	~MusicPlayer();

	void start();
	void stop();
	bool isPlaying() const { return mActive; }

	bool mix(Uint8* stream, int length);

private:
	MusicPlayer();

	bool openNextTrack();

	std::vector<std::string> mPlaylist;
	std::size_t mNext = 0;
	std::size_t mCurrent = 0;
	std::unique_ptr<MusicTrack> mTrack;
	bool mActive = false;
};

#endif // ES_CORE_MUSIC_PLAYER_H
