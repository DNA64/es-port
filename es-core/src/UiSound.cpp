/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "UiSound.h"

#include "Sound.h"
#include <string>

namespace UiSound
{
	void play(const char* file)
	{
		Sound::get(std::string("/media/data/emulationstation/audio/") + file)->play();
	}
}
