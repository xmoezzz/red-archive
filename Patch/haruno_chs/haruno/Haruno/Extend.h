#ifndef _Extend_
#define _Extend_

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
}
#include "SDL\SDL.h"
#include "SDL\SDL_mixer.h"

using namespace std;

int PlayMovie(const char *FileName);

int StartMovie();

#endif
