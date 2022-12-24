#ifndef _Application_
#define _Application_

#include "SDL\SDL.h"
#include "StartMovie.h"
#include "Renderer.h"
#include "Sound.h"


#include <Windows.h>

class Application
{
public:
	~Application();
	Application();


	bool initApplication();
	void runApplication();
private:
	SDL_Window* win;

	/****************/
	//sub system
	//��һ�� ����ȥдsub system manager

	Renderer* ren;
	MoviePlayer* player;
	Sound* sound;
};

#endif
