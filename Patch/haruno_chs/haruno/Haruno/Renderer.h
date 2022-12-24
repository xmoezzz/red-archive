#ifndef _Renderer_
#define _Renderer_

#include "SDL\SDL.h"

class Renderer
{
private:
	Renderer();

public:
	static Renderer* Handle;
	static Renderer* GetRenderer();


	SDL_Renderer* Get();
	~Renderer();
	bool init(SDL_Window* win);
	void Release();
private:

	SDL_Renderer* ren;
};

#endif
