#include "stdafx.h"
#include "StartMovie.h"
#include "Renderer.h"

MoviePlayer * MoviePlayer::Handle = NULL;

int MoviePlayer::sfp_refresh_thread(void *opaque)
{
	MoviePlayer* tThis = (MoviePlayer*)opaque;
	while (tThis->thread_exit == 0)
	{
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	return 0;
}

MoviePlayer * MoviePlayer::CetVideoHandle()
{
	if (Handle == NULL)
	{
		Handle = new MoviePlayer;
	}
	return Handle;
}

MoviePlayer::MoviePlayer() :
screen_w(0),
screen_h(0),
sdlTexture(NULL),
video_tid(NULL),
video_main_tid(NULL),
img_convert_ctx(NULL),
pFormatCtx(NULL),
i(0),
videoindex(0),
pCodecCtx(NULL),
pCodec(NULL),
pFrame(NULL),
pFrameYUV(NULL),
out_buffer(NULL),
packet(NULL),
got_picture(0),
thread_exit(0),
is_pausing(false),
is_playing(false),
Launched(false)
{

}

MoviePlayer::~MoviePlayer()
{
	if (img_convert_ctx)
		sws_freeContext(img_convert_ctx);
	if (pFrameYUV)
		av_frame_free(&pFrameYUV);
	if (pFrame)
		av_frame_free(&pFrame);
	if (pCodecCtx)
		avcodec_close(pCodecCtx);
	if (pFormatCtx)
		avformat_close_input(&pFormatCtx);
	is_playing = false;
	is_pausing = false;
	Launched = false;
	Handle = NULL;
}

bool MoviePlayer::Play(const char* filepath)
{
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	MessageBoxA(NULL, filepath, "", MB_OK);

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		return false;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
	{
		return false;
	}
	videoindex = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	if (videoindex == -1)
	{
		return false;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		return false;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
	{
		return false;
	}
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);


	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	Renderer* tRenderer = Renderer::GetRenderer();
	sdlTexture = SDL_CreateTexture(tRenderer->Get(), SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	//video_main_tid = SDL_CreateThread(MoviePlayer::Render, NULL, this);
	Render(this);
	Launched = true;
	return true;
}

int MoviePlayer::Render(void* This)
{
	MoviePlayer *tThis = (MoviePlayer*)This;
	tThis->video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, This);
	Renderer* tRenderer = Renderer::GetRenderer();
	SDL_Rect Draw;
	Draw.x = 0;
	Draw.y = 0;
	Draw.w = tThis->screen_w;
	Draw.h = tThis->screen_h;
	SDL_Event event;
	int ret;
	for (;;)
	{
		//Wait
		SDL_WaitEvent(&event);
		if (event.type == SFM_PAUSE_EVENT)
		{
			tThis->is_pausing = true;
		}
		else if (event.type == SFM_CONTINUE_EVENT)
		{
			tThis->is_pausing = false;
		}
		else if (event.type == SFM_REFRESH_EVENT && (!tThis->is_pausing))
		{
			//------------------------------
			if (av_read_frame(tThis->pFormatCtx, tThis->packet) >= 0)
			{
				if (tThis->packet->stream_index == tThis->videoindex)
				{
					ret = avcodec_decode_video2(tThis->pCodecCtx, tThis->pFrame, &(tThis->got_picture), tThis->packet);
					if (ret < 0)
					{
						return false;
					}
					if (tThis->got_picture)
					{
						sws_scale(tThis->img_convert_ctx, (const uint8_t* const*)tThis->pFrame->data,
							tThis->pFrame->linesize, 0, tThis->pCodecCtx->height, tThis->pFrameYUV->data, tThis->pFrameYUV->linesize);
						//SDL---------------------------
						SDL_UpdateTexture(tThis->sdlTexture, NULL, tThis->pFrameYUV->data[0], tThis->pFrameYUV->linesize[0]);
						SDL_RenderClear(Renderer::GetRenderer()->Get());
						SDL_RenderCopy(Renderer::GetRenderer()->Get(), tThis->sdlTexture, NULL, NULL);
					}
				}
				av_free_packet(tThis->packet);
			}
			else
			{
				//Exit Thread
				tThis->thread_exit = 1;
				break;
			}
		}
		else if (event.type == SDL_QUIT)
		{
			tThis->thread_exit = 1;
			break;
		}

	}
	return 0;
}

void MoviePlayer::Stop()
{
	thread_exit = 1;
	if (img_convert_ctx)
		sws_freeContext(img_convert_ctx);
	if (pFrameYUV)
		av_frame_free(&pFrameYUV);
	if (pFrame)
		av_frame_free(&pFrame);
	if (pCodecCtx)
		avcodec_close(pCodecCtx);
	if (pFormatCtx)
		avformat_close_input(&pFormatCtx);
	is_playing = false;
	is_pausing = false;
}

void MoviePlayer::Pause(int flag)
{
	//pause
	if (flag)
	{
		SDL_Event event;
		event.type = SFM_PAUSE_EVENT;
		SDL_PushEvent(&event);
	}
	else //continue
	{
		SDL_Event event;
		event.type = SFM_CONTINUE_EVENT;
		SDL_PushEvent(&event);
	}
}


bool MoviePlayer::isPlaying()
{
	return is_playing && (!is_pausing);
}

bool MoviePlayer::isLaunched()
{
	return Launched;
}
