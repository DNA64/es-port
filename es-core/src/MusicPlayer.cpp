/**
  * Copyright (c) 2026 DefKorns (https://defkorns.github.io/LICENSE)
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  */
#include "MusicPlayer.h"

#include "Log.h"
#include "Settings.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3_ex.h"

namespace
{
	constexpr const char* MusicDirectory = "/media/hakchi/music_menu";
	constexpr const char* FallbackTrack = "/media/data/emulationstation/audio/bgm_home.wav";
	constexpr int OutputRate = 44100;
	constexpr std::size_t BufferFrames = 2048;

	std::uint32_t readLittleEndian(const unsigned char* bytes)
	{
		return static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8) |
			(static_cast<std::uint32_t>(bytes[2]) << 16) | (static_cast<std::uint32_t>(bytes[3]) << 24);
	}

	bool hasExtension(const std::string& path, const char* extension)
	{
		return Utils::String::toLower(Utils::FileSystem::getExtension(path)) == extension;
	}

	class PcmSource
	{
	public:
		virtual ~PcmSource() = default;
		virtual std::size_t read(std::int16_t* destination, std::size_t frames) = 0;

		int rate = OutputRate;
		int channels = 2;
	};

	class WavSource : public PcmSource
	{
	public:
		bool open(const std::string& path)
		{
			mFile.reset(std::fopen(path.c_str(), "rb"));
			if(!mFile)
				return false;

			unsigned char header[12];
			if(std::fread(header, 1, sizeof header, mFile.get()) != sizeof header || std::memcmp(header, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0)
				return false;

			bool formatOk = false;
			unsigned char chunk[8];
			while(std::fread(chunk, 1, sizeof chunk, mFile.get()) == sizeof chunk)
			{
				const std::uint32_t size = readLittleEndian(chunk + 4);
				if(std::memcmp(chunk, "fmt ", 4) == 0 && size >= 16)
				{
					unsigned char format[16];
					if(std::fread(format, 1, sizeof format, mFile.get()) != sizeof format)
						return false;
					channels = format[2];
					rate = static_cast<int>(readLittleEndian(format + 4));
					formatOk = format[0] == 1 && (channels == 1 || channels == 2) && format[14] == 16 && rate > 0;
					std::fseek(mFile.get(), static_cast<long>(size) - 16 + static_cast<long>(size & 1), SEEK_CUR);
				}
				else if(std::memcmp(chunk, "data", 4) == 0)
				{
					mRemaining = size;
					return formatOk;
				}
				else
					std::fseek(mFile.get(), static_cast<long>(size) + static_cast<long>(size & 1), SEEK_CUR);
			}
			return false;
		}

		std::size_t read(std::int16_t* destination, std::size_t frames) override
		{
			const std::size_t frameBytes = static_cast<std::size_t>(channels) * 2;
			const std::size_t wanted = std::min<std::size_t>(frames * frameBytes, mRemaining);
			const std::size_t got = std::fread(destination, 1, wanted, mFile.get());
			mRemaining -= got;
			return got / frameBytes;
		}

	private:
		std::unique_ptr<FILE, int (*)(FILE*)> mFile { nullptr, &std::fclose };
		std::size_t mRemaining = 0;
	};

	class Mp3Source : public PcmSource
	{
	public:
		~Mp3Source() override
		{
			if(mOpen)
				mp3dec_ex_close(&mDecoder);
		}

		bool open(const std::string& path)
		{
			if(mp3dec_ex_open(&mDecoder, path.c_str(), MP3D_SEEK_TO_SAMPLE) != 0)
				return false;
			mOpen = true;
			rate = mDecoder.info.hz;
			channels = mDecoder.info.channels;
			return rate > 0 && (channels == 1 || channels == 2);
		}

		std::size_t read(std::int16_t* destination, std::size_t frames) override
		{
			const std::size_t samples = mp3dec_ex_read(&mDecoder, destination, frames * static_cast<std::size_t>(channels));
			return samples / static_cast<std::size_t>(channels);
		}

	private:
		mp3dec_ex_t mDecoder;
		bool mOpen = false;
	};
}

class MusicTrack
{
public:
	explicit MusicTrack(std::unique_ptr<PcmSource> source)
		: mSource(std::move(source)), mStep(static_cast<double>(mSource->rate) / OutputRate), mBuffer(BufferFrames * 2)
	{
	}

	std::size_t read(std::int16_t* output, std::size_t frames)
	{
		if(mSource->rate == OutputRate && mSource->channels == 2)
			return mSource->read(output, frames);

		std::size_t produced = 0;
		while(produced < frames)
		{
			const std::size_t index = static_cast<std::size_t>(mPosition);
			if(index + 1 >= mCount && !refill(index))
				break;

			const std::size_t base = static_cast<std::size_t>(mPosition);
			const double fraction = mPosition - static_cast<double>(base);
			for(int side = 0; side < 2; ++side)
			{
				const std::size_t channel = mSource->channels == 2 ? static_cast<std::size_t>(side) : 0;
				const double first = mBuffer[base * static_cast<std::size_t>(mSource->channels) + channel];
				const double second = mBuffer[(base + 1) * static_cast<std::size_t>(mSource->channels) + channel];
				output[produced * 2 + static_cast<std::size_t>(side)] = static_cast<std::int16_t>(first + (second - first) * fraction);
			}
			mPosition += mStep;
			++produced;
		}
		return produced;
	}

private:
	bool refill(std::size_t consumed)
	{
		const std::size_t channels = static_cast<std::size_t>(mSource->channels);
		const std::size_t keep = mCount > consumed ? mCount - consumed : 0;
		if(keep > 0)
			std::memmove(mBuffer.data(), mBuffer.data() + consumed * channels, keep * channels * sizeof(std::int16_t));
		mPosition -= static_cast<double>(consumed);
		mCount = keep;

		const std::size_t got = mSource->read(mBuffer.data() + mCount * channels, BufferFrames - mCount);
		mCount += got;
		return mCount >= 2 && static_cast<std::size_t>(mPosition) + 1 < mCount;
	}

	std::unique_ptr<PcmSource> mSource;
	double mStep;
	double mPosition = 0.0;
	std::size_t mCount = 0;
	std::vector<std::int16_t> mBuffer;
};

MusicPlayer::MusicPlayer() = default;

MusicPlayer::~MusicPlayer() = default;

MusicPlayer& MusicPlayer::get()
{
	static MusicPlayer player;
	return player;
}

void MusicPlayer::start()
{
	if(mActive || !Settings::getInstance()->getBool("MusicEnabled"))
		return;

	std::vector<std::string> playlist;
	if(Utils::FileSystem::isDirectory(MusicDirectory))
	{
		for(const std::string& path : Utils::FileSystem::getDirContent(MusicDirectory))
			if(hasExtension(path, ".wav") || hasExtension(path, ".mp3"))
				playlist.push_back(path);
		std::sort(playlist.begin(), playlist.end());
	}
	if(playlist.empty() && Utils::FileSystem::exists(FallbackTrack))
		playlist.push_back(FallbackTrack);
	if(playlist.empty())
	{
		LOG(LogInfo) << "MusicPlayer: no music found";
		return;
	}

	SDL_LockAudio();
	if(playlist != mPlaylist)
	{
		mPlaylist = std::move(playlist);
		mNext = 0;
	}
	mTrack.reset();
	mActive = true;
	SDL_UnlockAudio();
	LOG(LogInfo) << "MusicPlayer: " << mPlaylist.size() << " track(s), first " << mPlaylist.front();
	SDL_PauseAudio(0);
}

void MusicPlayer::stop()
{
	SDL_LockAudio();
	if(mTrack)
		mNext = mCurrent;
	mActive = false;
	mTrack.reset();
	SDL_UnlockAudio();
}

bool MusicPlayer::openNextTrack()
{
	for(std::size_t attempts = 0; attempts < mPlaylist.size(); ++attempts)
	{
		const std::string& path = mPlaylist[mNext];
		mCurrent = mNext;
		mNext = (mNext + 1) % mPlaylist.size();

		std::unique_ptr<PcmSource> source;
		if(hasExtension(path, ".mp3"))
		{
			std::unique_ptr<Mp3Source> mp3(new Mp3Source);
			if(mp3->open(path))
				source = std::move(mp3);
		}
		else
		{
			std::unique_ptr<WavSource> wav(new WavSource);
			if(wav->open(path))
				source = std::move(wav);
		}

		if(source)
		{
			mTrack.reset(new MusicTrack(std::move(source)));
			return true;
		}
	}
	return false;
}

bool MusicPlayer::mix(Uint8* stream, int length)
{
	if(!mActive)
		return false;

	const int volume = std::max(0, std::min(100, Settings::getInstance()->getInt("MusicVolume"))) * SDL_MIX_MAXVOLUME / 100;
	std::array<std::int16_t, 2048> buffer;
	const std::size_t frameBytes = 4;

	int done = 0;
	while(done < length)
	{
		if(!mTrack && !openNextTrack())
		{
			mActive = false;
			return done > 0;
		}

		const std::size_t frames = std::min<std::size_t>(buffer.size() / 2, static_cast<std::size_t>(length - done) / frameBytes);
		const std::size_t produced = mTrack->read(buffer.data(), frames);
		if(produced == 0)
		{
			mTrack.reset();
			continue;
		}

		SDL_MixAudio(stream + done, reinterpret_cast<const Uint8*>(buffer.data()), static_cast<Uint32>(produced * frameBytes), volume);
		done += static_cast<int>(produced * frameBytes);
	}

	return true;
}
