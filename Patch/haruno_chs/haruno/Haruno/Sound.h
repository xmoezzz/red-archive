#ifndef _Sound_
#define _Sound_

#include "SDL\SDL_mixer.h"

class Sound
{
public:
	Sound();
	~Sound();


	bool initSoundInterface();
	void Play(const char* filename, int fade = 0, bool loop = true);
	void Stop(int fade = 0);
private:

	Mix_Chunk* chunk;
};

#endif
