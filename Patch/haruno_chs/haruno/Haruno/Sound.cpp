#include "stdafx.h"
#include "Sound.h"


Sound::Sound() :
chunk(NULL)
{
	
}

Sound::~Sound()
{
	if (chunk)
	{
		Mix_FreeChunk(chunk);
		chunk = NULL;
	}
}

bool Sound::initSoundInterface()
{
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Init(MIX_INIT_OGG);
	Mix_AllocateChannels(2);
	return true;
}

void Sound::Play(const char* filename, int fade, bool loop)
{
	if (chunk)
	{
		this->Stop();
	}
	Mix_FreeChunk(chunk);
	chunk = Mix_LoadWAV(filename);
	if (chunk != NULL)
	{
		Mix_Volume(0, 128);
		Mix_FadeInChannel(0, chunk, loop == 1 ? -1 : 0, fade);
	}
}


void Sound::Stop(int fade)
{
	if (chunk && Mix_Playing(0))
	{
		Mix_FadeOutChannel(0, fade);
	}
}
