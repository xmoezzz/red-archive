#ifndef _Movie2_
#define _Movie2_

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#include "SDL/SDL.h"

#include <stdio.h>
#include <math.h>

int PlayMovie2(const char* FileName = NULL);

#endif
