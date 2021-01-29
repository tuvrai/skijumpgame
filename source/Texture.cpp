#include "Texture.h"
#include <SDL_ttf.h>

GameTexture::GameTexture()
{
	hTexture = NULL;
	width = 0;
	height = 0;
}

GameTexture::~GameTexture()
{
	free();
}

void GameTexture::free()
{
	if (hTexture != NULL)
	{
		SDL_DestroyTexture(hTexture);
		hTexture = NULL;
		width = 0;
		height = 0;
	}
}


int GameTexture::getWidth()
{
	return width;
}

int GameTexture::getHeight()
{
	return height;
}