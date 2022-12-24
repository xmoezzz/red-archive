#include "stdafx.h"
#include "Movie2.h"


#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

#define SDL_AUDIO_BUFFER_SIZE 1024

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)  //大小如何确定的???
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024) //大小如何确定的???

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)

#define VIDEO_PICTURE_QUEUE_SIZE 1


#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <queue>
using namespace std;

#pragma warning(disable: 4996)

#define SDL_AUDIO_BUFFER_SIZE            (1152)
#define AVCODEC_MAX_AUDIO_FRAME_SIZE    (192000)

#if (defined main && defined __MINGW__)
#undef main
#endif

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

static int64_t audio_pts = 0;
static int64_t audio_dts = 0;
static int64_t video_pts = 0;
static int64_t video_dts = 0;

static AVFrame *g_pFrameYUV;

SDL_Thread *g_pVideoThread;
SDL_mutex *g_pVideoMutex;

static int quit = 0;
static int video_quit = 0;

typedef struct video_thread_params
{
	SwsContext *sws_context;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	AVCodecContext *vid_codec_context;
	SDL_mutex *video_mutex;
}
video_thread_params;

int video_thread_proc(void *data);

void fill_audio(void *udata, Uint8 *stream, int len){
	if (audio_len == 0)
		return;
	len = (len > audio_len ? audio_len : len);
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int AudioResampling(AVCodecContext * audio_dec_ctx,
	AVFrame * pAudioDecodeFrame,
	int out_sample_fmt,
	int out_channels,
	int out_sample_rate,
	uint8_t* out_buf)
{
	SwrContext * swr_ctx = NULL;
	int data_size = 0;
	int ret = 0;
	int64_t src_ch_layout = audio_dec_ctx->channel_layout;
	int64_t dst_ch_layout = AV_CH_LAYOUT_STEREO;
	int dst_nb_channels = 0;
	int dst_linesize = 0;
	int src_nb_samples = 0;
	int dst_nb_samples = 0;
	int max_dst_nb_samples = 0;
	uint8_t **dst_data = NULL;
	int resampled_data_size = 0;

	swr_ctx = swr_alloc();
	if (!swr_ctx)
	{
		printf("swr_alloc error \n");
		return -1;
	}

	src_ch_layout = (audio_dec_ctx->channels ==
		av_get_channel_layout_nb_channels(audio_dec_ctx->channel_layout)) ?
		audio_dec_ctx->channel_layout :
		av_get_default_channel_layout(audio_dec_ctx->channels);

	if (out_channels == 1)
	{
		dst_ch_layout = AV_CH_LAYOUT_MONO;
		//printf("dst_ch_layout: AV_CH_LAYOUT_MONO\n");
	}
	else if (out_channels == 2)
	{
		dst_ch_layout = AV_CH_LAYOUT_STEREO;
		//printf("dst_ch_layout: AV_CH_LAYOUT_STEREO\n");
	}
	else
	{
		dst_ch_layout = AV_CH_LAYOUT_SURROUND;
		//printf("dst_ch_layout: AV_CH_LAYOUT_SURROUND\n");
	}

	if (src_ch_layout <= 0)
	{
		//printf("src_ch_layout error \n");
		return -1;
	}

	src_nb_samples = pAudioDecodeFrame->nb_samples;
	if (src_nb_samples <= 0)
	{
		//printf("src_nb_samples error \n");
		return -1;
	}

	av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", audio_dec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_dec_ctx->sample_fmt, 0);

	av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", (AVSampleFormat)out_sample_fmt, 0);

	if ((ret = swr_init(swr_ctx)) < 0) {
		//printf("Failed to initialize the resampling context\n");
		return -1;
	}

	max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples,
		out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
	if (max_dst_nb_samples <= 0)
	{
		//printf("av_rescale_rnd error \n");
		return -1;
	}

	dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
	ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels,
		dst_nb_samples, (AVSampleFormat)out_sample_fmt, 0);
	if (ret < 0)
	{
		//printf("av_samples_alloc_array_and_samples error \n");
		return -1;
	}


	dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, audio_dec_ctx->sample_rate) +
		src_nb_samples, out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
	if (dst_nb_samples <= 0)
	{
		//printf("av_rescale_rnd error \n");
		return -1;
	}
	if (dst_nb_samples > max_dst_nb_samples)
	{
		av_free(dst_data[0]);
		ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
			dst_nb_samples, (AVSampleFormat)out_sample_fmt, 1);
		max_dst_nb_samples = dst_nb_samples;
	}

	if (swr_ctx)
	{
		ret = swr_convert(swr_ctx, dst_data, dst_nb_samples,
			(const uint8_t **)pAudioDecodeFrame->data, pAudioDecodeFrame->nb_samples);
		if (ret < 0)
		{
			//printf("swr_convert error \n");
			return -1;
		}

		resampled_data_size = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
			ret, (AVSampleFormat)out_sample_fmt, 1);
		if (resampled_data_size < 0)
		{
			//printf("av_samples_get_buffer_size error \n");
			return -1;
		}
	}
	else
	{
		//printf("swr_ctx null error \n");
		return -1;
	}

	memcpy(out_buf, dst_data[0], resampled_data_size);

	if (dst_data)
	{
		av_freep(&dst_data[0]);
	}
	av_freep(&dst_data);
	dst_data = NULL;

	if (swr_ctx)
	{
		swr_free(&swr_ctx);
	}
	return resampled_data_size;
}

//创建一个全局的结构体变量以便于我们从文件中得到的声音包有地方存
//放同时也保证SDL中的声音回调函数audio_callback 能从这个地方得到声音数据
typedef struct PacketQueue{
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;//因为SDL 是在一个独立的线程中来进行音频处理的。如果我们没有正确的锁定这个队列，我们有可能把数据搞乱。
	SDL_cond *cond;
}PacketQueue;

PacketQueue audioq;
PacketQueue videoq;
queue<AVFrame *> frameq;

void packet_queue_init(PacketQueue *pq){
	memset(pq, 0, sizeof(PacketQueue));
	pq->mutex = SDL_CreateMutex();
	pq->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt){
	AVPacketList *pkt1;
	/*(
	if (av_dup_packet(pkt) < 0){
	printf("error");
	return -1;
	}
	*/

	pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
	if (!pkt1){
		printf("error");
		return -1;
	}

	av_copy_packet(&pkt1->pkt, pkt);
	av_free_packet(pkt);
	pkt1->next = NULL;

	//函数SDL_LockMutex()锁定队列的互斥量以便于我们向队列中添加东西，然后函
	//数SDL_CondSignal()通过我们的条件变量为一个接 收函数（如果它在等待）发
	//出一个信号来告诉它现在已经有数据了，接着就会解锁互斥量并让队列可以自由
	//访问。
	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)//队列为空
		q->first_pkt = pkt1;
	else//队列不为空
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);

	return 0;
}

int decode_interrupt_cb(void){
	return quit;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block){
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;){
		if (quit){
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1){
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			//*pkt = pkt1->pkt;
			av_copy_packet(pkt, &pkt1->pkt);
			av_free_packet(&pkt1->pkt);
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block){
			ret = 0;
			break;
		}
		else{
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size){
	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;

	int len1, data_size, ret = 0;

	static AVFrame *pFrame;
	pFrame = av_frame_alloc();

	/*if (packet_queue_get(&audioq, &pkt, 1) < 0){//从这里开始，取得main线程放入队列的包
	printf("error, can't get packet from the queue");
	return -1;
	}

	len1 = avcodec_decode_audio4(aCodecCtx, pFrame, &ret, &pkt);
	if (len1 < 0)
	return -1;

	return AudioResampling(aCodecCtx, pFrame, AV_SAMPLE_FMT_S16, 2, 44100, audio_buf);*/
	for (;;){
		while (audio_pkt_size > 0){
			data_size = buf_size;
			len1 = avcodec_decode_audio4(aCodecCtx, pFrame, &ret, &pkt);

			//len1 = avcodec_decode_audio3(aCodecCtx, (int16_t *)audio_buf,
			//    &data_size, &pkt);
			if (len1 < 0){//if error, skip frame
				printf("error\n");
				audio_pkt_size = 0;
				break;
			}
			data_size = AudioResampling(aCodecCtx, pFrame, AV_SAMPLE_FMT_S16, 2, 44100, audio_buf);
			audio_pkt_data += len1;
			audio_pkt_size -= len1;
			if (data_size <= 0)//No data yet, get more frames
				continue;
			return data_size;
		}
		//if (pkt.data)
		av_free_packet(&pkt);
		if (quit)
			return -1;
		if (packet_queue_get(&audioq, &pkt, 1) < 0){//从这里开始，取得main线程放入队列的包
			printf("error, can't get packet from the queue");
			return -1;
		}

		//SDL_LockMutex(g_pVideoMutex);
		audio_pts = pkt.pts;
		audio_dts = pkt.dts;
		//SDL_UnlockMutex(g_pVideoMutex);

		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}
}
//声音回调函数
//userdata是输入，stream是输出，len是输入，len的值一般为4096（调试中发现的），
//audio_callback函数的功能是调用audio_decode_frame函数，把解码后数据块audio_buf追加在stream的后面，
//通过SDL库对audio_callback的不断调用，不断解码数据，然后放到stream的末尾，
//SDL库认为stream中数据够播放一帧音频了，就播放它, 
//第三个参数len是向stream中写数据的内存分配尺度，是分配给audio_callback函数写入缓存大小。
void audio_callback(void *userdata, Uint8 *stream, int len){
	//SDL_memset(stream, 0, len);
	AVCodecContext *aCodecCtx = (AVCodecContext*)userdata;
	int len1, audio_size;

	//audio_buf 的大小为 1.5 倍的声音帧的大    小以便于有一个比较好的缓冲
	static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	while (len > 0){
		if (audio_buf_index >= audio_buf_size){//already send all our data, get more
			audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
			if (audio_size < 0){//error, output silence
				//printf("error, output silence\n");
				audio_buf_size = SDL_AUDIO_BUFFER_SIZE;
				memset(audio_buf, 0, audio_buf_size);
			}
			else
				audio_buf_size = audio_size;
			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if (len1>len){
			len1 = len;
		}
		memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
	}
}

// 视频流处理线程
// 这里对齐是以音频流为准的
// 所以如果音频流（相对于视频流）过于稀疏的话，就会失去作用，效果不好

int video_thread_proc(void *data)
{
	video_thread_params *params = (video_thread_params *)data;
	AVFrame *pFrame = NULL;
	AVFrame *pNextFrame = NULL;
	AVPacket packet = { 0 };
	AVPacket nextpacket;

	// 注意，以下代码比较的都是 DTS（解压缩时间戳）而不是 PTS（显示时间戳）！！！
	// 实际视频显示是以 PTS 为准的，但是 PTS 大多没有规律（只要是在解压之后就好），难以处理
	// 所幸在相当一部分视频中 DTS 与 PTS 相差较小，所以这里就用 DTS 了

	while (!video_quit)
	{
		while (!frameq.empty())
		{
			if (pFrame == NULL)
			{
				SDL_LockMutex(params->video_mutex);

				// 这里采用了“连续读取”的方法，也即如果下一帧的 DTS 小于当前基准（目前采用音频流），则循环直至找到第一个 DTS 比基准大的
				// 然后播放小于基准而且离基准最近的帧，并缓存下一帧
				// 由于处理时间较长，而且使用了互斥体（看，音频部分也用到了），所以可能有极少数的音频帧有异常（噪声啊之类的）
				// 当然也可以将这一段注释掉，采用 pFrame = frameq.count(); frameq.pop();，同时解除 if (packet_queue_get()) 的注释（和下面的一对大括号）
				// 但是无跳过的方法会导致视频严重滞后

				if (pNextFrame != NULL)
				{
					pFrame = pNextFrame;
					SDL_memcpy(&packet, &nextpacket, sizeof(AVPacket));
					pNextFrame = NULL;
				}
				else
				{
					pFrame = frameq.front();
					frameq.pop();
				}
				while (!frameq.empty())
				{
					pNextFrame = frameq.front();
					frameq.pop();
					packet_queue_get(&videoq, &nextpacket, 1);

					if (nextpacket.dts <= audio_dts)
					{
						av_free_packet(&packet);
						av_frame_free(&pFrame);
						SDL_memcpy(&packet, &nextpacket, sizeof(AVPacket));
						pFrame = pNextFrame;
						pNextFrame = NULL;
					}
					else
					{
						break;
					}
				}

				SDL_PumpEvents();
				//pFrame = frameq.front();
				//frameq.pop();


				//cout << "vdts: " << packet.dts << " adts: " << audio_dts << endl;

				//if (packet_queue_get(&videoq, &packet, 1) >= 0)
				//{
				sws_scale(params->sws_context, (const uint8_t* const*)pFrame->data,
					pFrame->linesize, 0, params->vid_codec_context->height, g_pFrameYUV->data, g_pFrameYUV->linesize);

				SDL_UpdateYUVTexture(params->texture, NULL, g_pFrameYUV->data[0], g_pFrameYUV->linesize[0],
					g_pFrameYUV->data[1], g_pFrameYUV->linesize[1], g_pFrameYUV->data[2], g_pFrameYUV->linesize[2]);

				//SDL_RenderClear(params->renderer);
				SDL_RenderCopy(params->renderer, params->texture, NULL, NULL);
				SDL_RenderPresent(params->renderer);
				// 可以使用 av_frame_clone() + 队列 实现“远程”dts 读取
				//if (params->vid_codec_context->refcounted_frames)
				//{
				//    av_frame_unref(pFrame);
				//}
				//}

				//cout << "--------------------------------------------------" << endl;
				//cout << "vidpts: " << packet.pts << " audpts: " << audio_pts << endl;
				//cout << "viddts: " << packet.dts << " auddts: " << audio_dts << endl;

				SDL_UnlockMutex(params->video_mutex);
			}
			else
			{
				//cout << "vdts: " << packet.dts << " adts: " << audio_dts << endl;

				// 如果当前帧应该用被缓存的帧，那就用，不要重新读取了

				sws_scale(params->sws_context, (const uint8_t* const*)pFrame->data,
					pFrame->linesize, 0, params->vid_codec_context->height, g_pFrameYUV->data, g_pFrameYUV->linesize);

				SDL_UpdateYUVTexture(params->texture, NULL, g_pFrameYUV->data[0], g_pFrameYUV->linesize[0],
					g_pFrameYUV->data[1], g_pFrameYUV->linesize[1], g_pFrameYUV->data[2], g_pFrameYUV->linesize[2]);

				//SDL_RenderClear(params->renderer);
				SDL_RenderCopy(params->renderer, params->texture, NULL, NULL);
				SDL_RenderPresent(params->renderer);
				// 可以使用 av_frame_clone() + 队列 实现“远程”dts 读取
				if (params->vid_codec_context->refcounted_frames)
				{
					av_frame_unref(pFrame);
				}

				// 如果该帧是在音频帧之前的，那就销毁它，和数据包
				if (packet.dts <= audio_dts)
				{
					av_frame_free(&pFrame);
					av_free_packet(&packet);
					pFrame = NULL;
				}
			}

		}
	}

	return 0;
}

int PlayMovie2(const char* FileName)
{
	av_register_all();    //注册了所有的文件格式和编解码的库，它们将被自动的使用在被打开的合适格式的文件上
	AVFormatContext *pFormatCtx;
	pFormatCtx = avformat_alloc_context();

	//Open an input stream and read the header
	if (avformat_open_input(&pFormatCtx, FileName, NULL, NULL) != 0){
		//printf("Can't open the file\n");
		//return -1;
		MessageBoxW(NULL, L"输入失败", L"Maisakura min core", MB_OK);
		return -1;
	}
	//Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0){
		//printf("Couldn't find stream information.\n");
		//return -1;
		MessageBoxW(NULL, L"输入流失败", L"Maisakura min core", MB_OK);
		return -1;
	}



	int i, videoIndex, audioIndex;

	//Find the first video stream
	videoIndex = -1;
	audioIndex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++){//视音频流的个数
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
			&& videoIndex < 0){
			videoIndex = i;
		}
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
			&& audioIndex < 0)
			audioIndex = i;
	}

	if (videoIndex == -1)
		return -1;
	if (audioIndex == -1)
		return -1;

	AVCodecContext *pCodecCtx, *paCodecCtx;
	AVCodec *pCodec, *paCodec;
	//Get a pointer to the codec context for the video stream
	//流中关于编解码器的信息就是被我们叫做"codec context"（编解码器上下文）
	//的东西。这里面包含了流中所使用的关于编解码器的所有信
	pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
	// 帧引用：打开，见 AVFrame 的注释
	pCodecCtx->refcounted_frames = 1;
	paCodecCtx = pFormatCtx->streams[audioIndex]->codec;
	//Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	paCodec = avcodec_find_decoder(paCodecCtx->codec_id);

	if (pCodec == NULL || paCodecCtx == NULL)
	{
		//printf("Unsupported codec!\n");
		//return -1;
		MessageBoxW(NULL, L"视频流格式有误", L"Maisakura min core", MB_OK);
		return -1;
	}
	//Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
		//printf("Could not open video codec.\n");
		//return -1;
		MessageBoxW(NULL, L"无法创建视频解码器", L"Maisakura min core", MB_OK);
		return -1;
	}
	if (avcodec_open2(paCodecCtx, paCodec, NULL) < 0){
		//printf("Could not open audio codec.\n");
		//return -1;
		MessageBoxW(NULL, L"无法创建音频解码器", L"Maisakura min core", MB_OK);
	}


	//allocate video frame and set its fileds to default value
	AVFrame *pFrame;
	//AVFrame *pFrameYUV;
	pFrame = av_frame_alloc();
	g_pFrameYUV = av_frame_alloc();

	//即使我们申请了一帧的内存，当转换的时候，我们仍然需要一个地方来放置原始
	//的数据。我们使用avpicture_get_size 来获得我们需要的大小， 然后手工申请
	//内存空间：
	uint8_t *out_buffer;
	int numBytes;
	numBytes = avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	//av_malloc 是ffmpeg 的malloc，用来实现一个简单的malloc 的包装，这样来保
	//证内存地址是对齐的（4 字节对齐或者2 字节对齐）。它并不能保 护你不被内
	//存泄漏，重复释放或者其它malloc 的问题所困扰。
	out_buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	//Assign appropriate parts of buffer to image planes in pFrameYUV
	//Note that pFrameYUV is an AVFrame, but AVFrame is a superset of AVPicture
	avpicture_fill((AVPicture*)g_pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

	//----------------SDL--------------------------------------//
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)){
		MessageBoxW(NULL, L"内部启动失败", L"Maisakura min core", MB_OK);
		exit(1);
	}

	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = paCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = paCodecCtx->channels;    //声音的通道数
	wanted_spec.silence = 0;    //用来表示静音的值
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;    //声音缓冲区的大小
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = paCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL) < 0){
		//printf("SDL_OpenAudio error: %s\n", SDL_GetError());
		//return -1;
		MessageBoxW(NULL, L"音频驱动启动失败", L"Maisakura min core", MB_OK);
	}

	packet_queue_init(&audioq);
	packet_queue_init(&videoq);
	SDL_PauseAudio(0);

	SDL_Window *window = nullptr;

	wchar_t names[] = L"[X'moe汉化组]晴霁之后、定是菜花盛开的好天气 汉化特典";
	int nCount = WideCharToMultiByte(CP_UTF8, 0, names, -1, NULL, 0, NULL, NULL);
	char* pStr = new char[nCount];
	nCount = WideCharToMultiByte(CP_UTF8, 0, names, -1, pStr, nCount, NULL, NULL);

	window = SDL_CreateWindow(pStr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 600, SDL_WINDOW_SHOWN);
	if (!window){
		cout << SDL_GetError() << endl;
		return 1;
	}

	SDL_Renderer *ren = nullptr;
	ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr){
		cout << SDL_GetError() << endl;
		return -1;
	}

	SDL_Texture *texture = nullptr;
	texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_YV12,
		SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	//*************************************************************//
	//通过读取包来读取整个视频流，然后把它解码成帧，最后转换格式并且保存
	int frameFinished;
	//int psize = pCodecCtx->width * pCodecCtx->height;
	AVPacket packet;
	av_new_packet(&packet, numBytes);

	i = 0;
	int ret;
	static struct SwsContext *img_convert_ctx;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);

	SDL_Event ev;

	video_thread_params vtp;
	vtp.renderer = ren;
	vtp.texture = texture;
	vtp.sws_context = img_convert_ctx;
	vtp.vid_codec_context = pCodecCtx;
	vtp.video_mutex = SDL_CreateMutex();
	g_pVideoMutex = vtp.video_mutex;
	g_pVideoThread = SDL_CreateThread(video_thread_proc, "video_thread", &vtp);

	double v_a_ratio;            // 视频帧数/音频帧数
	int frame_queue_size;

	//Read the next frame of a stream
	while ((!quit) && (av_read_frame(pFormatCtx, &packet) >= 0 || (!frameq.empty())))
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
					/*
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height, g_pFrameYUV->data, g_pFrameYUV->linesize);

					SDL_UpdateYUVTexture(texture, NULL, g_pFrameYUV->data[0], g_pFrameYUV->linesize[0],
					g_pFrameYUV->data[1], g_pFrameYUV->linesize[1], g_pFrameYUV->data[2], g_pFrameYUV->linesize[2]);

					AVFrame *pNewFrame = av_frame_clone(pFrame);
					frameq.push(pNewFrame);
					packet_queue_put(&videoq, &packet);

					SDL_RenderClear(ren);
					SDL_RenderCopy(ren, texture, NULL, NULL);
					SDL_RenderPresent(ren);
					// 可以使用 av_frame_clone() + 队列 实现远程 dts 读取
					if (pCodecCtx->refcounted_frames)
					{
					av_frame_unref(pFrame);
					}
					cout << "<<<<<<" << endl;
					cout << "vpts: " << packet.pts << ", apts: " << audio_pts << endl;
					cout << "vdts: " << packet.dts << ", adts: " << audio_dts << endl;
					*/
					// 用互斥体保持同步，将包送入队列，让另外的线程自己处理
					SDL_PumpEvents();
					SDL_LockMutex(vtp.video_mutex);
					packet_queue_put(&videoq, &packet);
					AVFrame *pNewFrame = av_frame_clone(pFrame);
					frameq.push(pNewFrame);
					//cout << "Pushing vpacket." << endl;
					SDL_UnlockMutex(vtp.video_mutex);
				}
				// 注意这里也必须要 free packet，否则会导致严重内存泄露
				// 我修改了 packet_queue_put() 函数，它会复制 packet，所以可以放心释放上
				av_free_packet(&packet);
			}
			else{
				av_free_packet(&packet);
				//cout << "decode error" << endl;
				return -1;
			}
		}
		else if (packet.stream_index == audioIndex){
			//packet_queue_put(&audioq, &packet);
			/*ret = avcodec_decode_audio4(paCodecCtx, pFrame, &frameFinished, &packet);
			cout << pFrame->format << endl;

			if (ret < 0){
			printf("Error in decoding audio frame\n");
			exit(0);
			}
			if (frameFinished){
			printf("pts %5d\n", packet.pts);
			printf("dts %5d\n", packet.dts);
			printf("packet_size %5d\n", packet.size);
			}
			audio_chunk = (Uint8*)pFrame->data[0];
			audio_len = pFrame->linesize[0];

			audio_pos = audio_chunk;
			//SDL_PauseAudio(0);
			while (audio_len>0)
			SDL_Delay(1);*/
			packet_queue_put(&audioq, &packet);
		}
		else
		{
			av_free_packet(&packet);
		}

	process_sdl_events:
		if (SDL_PollEvent(&ev))
		{
			switch (ev.type){
			case SDL_QUIT:
			{
							 quit = 1;
							 video_quit = 1;
							 SDL_Quit();
							 goto exit_line;
							 break;
			}
			case SDL_KEYDOWN:
				if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					quit = 1;
					video_quit = 1;
					SDL_Quit();
					goto exit_line;
					break;
				}
			default:
				break;
			}
		}

		//cout << "vframes: " << pCodecCtx->frame_number << " aframes: " << paCodecCtx->frame_number << endl;

		// 设置一个缓冲区大小，如果超过此大小则暂停处理（视频和音频）帧
		// 这里采用限制 frameq 的大小的方法
		// 如果采用以下的动态代码，有可能导致结尾“崩坏”，囧
		/*
		if (paCodecCtx->frame_number == 0)
		{
		v_a_ratio = 300;        // 一个很大的值，基本上能保证所有视频都能解码
		}
		else
		{
		v_a_ratio = pCodecCtx->frame_number / (double)paCodecCtx->frame_number;
		if (v_a_ratio < 10.0) v_a_ratio = 10.0;
		}

		frame_queue_size = (int)v_a_ratio * 2;
		if (frameq.size() > frame_queue_size) goto process_sdl_events;
		*/
		if (frameq.size() > 50) goto process_sdl_events;
	}

exit_line:

	SDL_DestroyMutex(vtp.video_mutex);

	SDL_DestroyTexture(texture);

	av_frame_free(&pFrame);
	av_frame_free(&g_pFrameYUV);

	avcodec_close(pCodecCtx);

	avformat_close_input(&pFormatCtx);

	return 0;
}
