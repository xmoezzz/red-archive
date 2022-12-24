#include "stdafx.h"
#include "Renderer.h"

Renderer* Renderer::Handle = NULL;

Renderer::Renderer() :
ren(NULL)
{

}

Renderer::~Renderer()
{
	if (ren)
	{
		SDL_DestroyRenderer(ren);
		ren = NULL;
	}
}


Renderer* Renderer::GetRenderer()
{
	if (Handle == NULL)
	{
		Handle = new Renderer;
	}
	return Handle;
}

bool Renderer::init(SDL_Window* win)
{
	if (!ren)
	{
		SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	}
	return ren != NULL;
}

void Renderer::Release()
{
	if (ren)
	{
		SDL_DestroyRenderer(ren);
		ren = NULL;
	}
}


SDL_Renderer* Renderer::Get()
{
	return ren;
}
