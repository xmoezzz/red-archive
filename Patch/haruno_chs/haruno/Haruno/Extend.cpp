#include "stdafx.h"
#include "Extend.h"


static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;

void  fill_audio(void *udata, Uint8 *stream, int len)
{
	if (audio_len == 0)
		return;

	len = (len>audio_len ? audio_len : len);
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

static char *PlayList2[] =
{
	"mov\\fycho.mp4",
	"mov\\kaedeka.mp4",
	"mov\\zweib.mp4",
	"mov\\gr.mp4",
	"mov\\sammy.mp4",
	"mov\\Biscuits.mp4",
	NULL
};

int GetLength2()
{
	int ret = 0;
	while (PlayList2[ret])
	{
		ret++;
	}
	return ret;
}

int StartMovie()
{
	srand((int)time(0));
	unsigned int seed = (unsigned int)rand();

	seed %= (GetLength2() - 1);
	if (PlayList2[seed])
	{
		return PlayMovie(PlayList2[seed]);
	}
	else
		return 0;
}


int PlayMovie(const char *FileName)
{
	av_register_all();
	AVFormatContext *pFormatCtx;
	pFormatCtx = avformat_alloc_context();

	const char *filepath = FileName;
	//Open an input stream and read the header
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		return -1;
	}
	//Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		return -1;
	}

	int i, videoIndex;
	int audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;

	//Find the first video stream
	videoIndex = -1;
	audioStream = -1;
	bool VOk = false;
	bool AOk = false;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			break;
		}
	}
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
			break;
		}
	}
	if (videoIndex == -1)
	{
		return -1;
	}

	AVCodecContext* pCodecCtx2 = pFormatCtx->streams[audioStream]->codec;

	// Find the decoder for the audio stream  
	AVCodec* pCodec2 = avcodec_find_decoder(pCodecCtx2->codec_id);
	if (pCodec == NULL)
	{
	}

	// Open codec  
	if (avcodec_open2(pCodecCtx2, pCodec2, NULL)<0)
	{
	}

	pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		return -1;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		return -1;
	}

	AVFrame *pFrame, *pFrameYUV;
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();


	uint8_t *out_buffer;
	int numBytes;
	numBytes = avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	out_buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture*)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);


	//----------------SDL--------------------------------------//
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_AUDIO))
	{
		MessageBoxW(NULL, L"内部启动失败", L"Maisakura min core", MB_OK);
		exit(1);
	}

	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = pCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = pCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = 1024; //播放AAC，M4a，缓冲区的大小  
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL)<0)//步骤（2）打开音频设备   
	{
		//printf("can't open audio.\n");
		//return 0;
		MessageBoxW(NULL, L"无法打开音频设备", L"Maidakura min core", MB_OK);
	}

	SDL_Window *window = nullptr;

	wchar_t names[] = L"[X'moe汉化组]晴霁之后、定是菜花盛开的好天气 汉化特典";
	int nCount = WideCharToMultiByte(CP_UTF8, 0, names, -1, NULL, 0, NULL, NULL);
	char* pStr = new char[nCount];
	nCount = WideCharToMultiByte(CP_UTF8, 0, names, -1, pStr, nCount, NULL, NULL);

	window = SDL_CreateWindow(pStr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 600, SDL_WINDOW_SHOWN);
	if (!window)
	{
		return 1;
	}

	SDL_Renderer *ren = nullptr;
	ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr)
	{
		return -1;
	}

	SDL_Texture *texture = nullptr;
	texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_YV12,
		SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
	SDL_Rect rect;
	rect.x = 0, rect.y = 0;
	rect.w = pCodecCtx->width;
	rect.h = pCodecCtx->height;

	int frameFinished;
	//int psize = pCodecCtx->width * pCodecCtx->height;
	AVPacket packet;
	av_new_packet(&packet, numBytes);


	i = 0;
	int ret;
	int ret2;
	static struct SwsContext *img_convert_ctx;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);

	int got_picture;
	int index = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		//Is this a packet from the video stream?
		if (packet.stream_index == videoIndex)
		{
			//decode video frame of size packet.size from packet.data into picture
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			//Did we get a video frame?
			if (ret >= 0)
			{
				//Convert the image from its native format to YUV
				if (frameFinished)
				{
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

					SDL_UpdateYUVTexture(texture, &rect, pFrameYUV->data[0], pFrameYUV->linesize[0],
						pFrameYUV->data[1], pFrameYUV->linesize[1], pFrameYUV->data[2], pFrameYUV->linesize[2]);

					SDL_PumpEvents();
					SDL_Event event;

					SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
					switch (event.type)
					{
					case SDL_QUIT:
						return 0;
					}

					SDL_RenderClear(ren);
					SDL_RenderCopy(ren, texture, &rect, &rect);
					SDL_RenderPresent(ren);
				}
				SDL_Delay(50);
			}
			else
			{
				return -1;
			}
		}
		else if (packet.stream_index == audioStream)
		{
			ret2 = avcodec_decode_audio4(pCodecCtx, pFrame,
				&got_picture, &packet);
			if (ret2 < 0) // if error len = -1  
			{
				if (got_picture > 0)
				{
					audio_chunk = (Uint8*)pFrame->data[0];
					audio_len = pFrame->linesize[0];
					audio_pos = audio_chunk;
					SDL_PauseAudio(0);
					index++;
				}
			}
		}
	}

	av_free_packet(&packet);

	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type)
	{
	case SDL_QUIT:
		SDL_Quit();
		exit(0);
		break;
	default:
		break;
	}

	SDL_DestroyTexture(texture);

	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);

	avcodec_close(pCodecCtx);

	avformat_close_input(&pFormatCtx);

	return 0;
}

