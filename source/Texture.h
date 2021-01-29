#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
//Class to manage textures
class GameTexture
{
private:
	//hardware texture
	SDL_Texture* hTexture;
	//texture width
	int width;
	//texture height
	int height;

public:
	//Initializer
	GameTexture();

	//Destroyer
	~GameTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

	//Destroy texture
	void free();

	//Renders texture at given point, with optional rotation and clip
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	//get texture width
	int getWidth();
	//get texture height
	int getHeight();

	//creates texture basing on text
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor, int size);
};