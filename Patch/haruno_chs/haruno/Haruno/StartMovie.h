#ifndef _StartMovie_
#define _StartMovie_

#include "SDL\SDL.h"
#include <stdio.h>

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
#include "SDL/SDL.h"
};

//Refresh
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_PAUSE_EVENT  (SDL_USEREVENT + 2)
#define SFM_CONTINUE_EVENT  (SDL_USEREVENT + 3)

class MoviePlayer
{
private:
	MoviePlayer();
	static MoviePlayer* Handle;

public:
	~MoviePlayer();
	static MoviePlayer *CetVideoHandle();

	bool Play(const char *path);
	void Stop();
	void Pause(int flag);
	bool isPlaying();
	bool isLaunched();

private:

	static int Render(void* This);
	static int sfp_refresh_thread(void *opaque);

	int screen_w, screen_h;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Thread *video_main_tid;

	struct SwsContext *img_convert_ctx;
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame, *pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int got_picture;

	bool is_playing;
	bool is_pausing;
	bool Launched;
	int thread_exit;
};

#endif
