#include "stdafx.h"
#include "Application.h"
//#include <cstdlib>

static char *PlayList[] = 
{
	"mov\\fycho.mp4",
	"mov\\kaedeka.mp4",
	"mov\\zweib.mp4",
	"mov\\gr.mp4",
	"mov\\sammy.mp4",
	"mov\\Biscuits.mp4",
	NULL
};

int GetLength()
{
	int ret = 0;
	while (PlayList[ret])
	{
		ret++;
	}
	return ret;
}

Application::Application() :
win(NULL),
ren(NULL),
player(NULL),
sound(NULL)
{

}

Application::~Application()
{
	//
	if (sound)
	{
		delete sound;
		sound = NULL;
	}
	if (win)
	{
		SDL_DestroyWindow(win);
		win = NULL;
	}
	if (ren)
	{
		ren->Release();
		ren = NULL;
	}
}

bool Application::initApplication()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		MessageBoxW(NULL, L"内部启动失败", L"Maisakura min core", MB_OK);
		return false;
	}

	wchar_t names[] = L"[X'moe汉化组]晴霁之后、定是菜花盛开的好天气 汉化特典";
	int nCount = WideCharToMultiByte(CP_UTF8, 0, names, -1, NULL, 0, NULL, NULL);
	char* pStr = new char[nCount];
	nCount = WideCharToMultiByte(CP_UTF8, 0, names, -1, pStr, nCount, NULL, NULL);

	win = SDL_CreateWindow(pStr,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1024,
		600,
		SDL_WINDOW_SHOWN);
	if (win == NULL)
	{
		MessageBoxW(NULL, L"无法为主程序创建窗口", L"Maisakura min core", MB_OK);
		return  false;
	}
	ren = Renderer::GetRenderer();
	if (ren == NULL)
	{
		MessageBoxW(NULL, L"图形驱动初始化失败", L"Maisakura min core", MB_OK);
		return  false;
	}
	
	ren->init(win);
	sound = new Sound();
	if (sound == NULL)
	{
		MessageBoxW(NULL, L"音频驱动初始化失败", L"Maidakura min core", MB_OK);
		//return true;//no sound driver supported
	}
	if (sound)
	{
		if (!sound->initSoundInterface())
		{
			MessageBoxW(NULL, L"音频接口初始化失败", L"Maidakura min core", MB_OK);
		}
	}
	return true;
}

int
rand_r(unsigned int *seed)
{
	unsigned int next = *seed;
	int result;

	next *= 1103515245;
	next += 12345;
	result = (unsigned int)(next / 65536) % 2048;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int)(next / 65536) % 1024;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int)(next / 65536) % 1024;

	*seed = next;

	return result;
}

void Application::runApplication()
{
	player = MoviePlayer::CetVideoHandle();
	if (!player)
	{
		MessageBoxW(NULL, L"视频接口初始化失败", L"Maisakura min core", MB_OK);
		return;
	}
	//srand(time(0));
	//unsigned int seed = (unsigned int)rand();
	unsigned int tmp = SDL_GetTicks();
	unsigned int seed = (unsigned int)rand_r(&tmp);
		seed %= (GetLength() - 1);
	if (PlayList[seed])
	{
		player->Play(PlayList[seed]);
	}
	while (1);
}
