//SDL (and other external libraries if used)
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
//standard libraries
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <random>
#include <conio.h>
#include <cstdio>

//internal objects
#include "Jumper.h"
#include "Texture.h"
#include "Wind.h"

using namespace std;

float randwind();
void setGate(int gate);
void createSpeedText(float speed);
void createDistanceText(float distance);
void createPlayerName(string name);
int getFontSizeRelativeToResolutionByConstant(float fontconst);
string trimzero(string text);
int randBetween(int a, int b);

bool checkName(string name);
string parsePlayerArrayToText(string arr[], int start, int end, int maxsize);
int countPlayers();

//Event handler
SDL_Event e;

//===========================TOURNAMENT VARIABLES ===================================
//list players who jump in the tournament. Max is 30 for now.
string players[30];

//distances of players in tournament
float distances[30];



//===============SDL - BASE AND WINDOW SETTINGS =====================

//SCREEN DIMENSIONS - RECOMMENDED IS FORMAT 16:9 (WIDTH:HEIGHT) SINCE THE FONT SIZE IS BUILT ON THOSE, OTHER FORMATS ARE AVAILABLE AND MOSTLY FINE LOOKING AS WELL THOUGH.

//width of screen window, set to default, recommended is below 1601 and above 400
const int SCREEN_WIDTH = 1024;
//height of screen window, set to default, recommended is below 901 and above 400
const int SCREEN_HEIGHT = 768;

//Starts up SDL and creates window
bool init();
//Loads media
bool loadMedia();
//Gets rid of all media and SDL modules, possibly dynamically created stuff as well
void close();
//Draws circle
void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}
//The window object where rendering happens
SDL_Window* gameWindow = NULL;
//The window renderer
SDL_Renderer* gameRenderer = NULL;

//================ TEXTURES TO RENDER ===========================
//menu textures
GameTexture menu_title;
GameTexture menu_opt1;
GameTexture menu_opt2;
GameTexture menu_opt3;
GameTexture menu_opt4;
GameTexture caption;
//jumper textures
GameTexture jumperTex;
GameTexture jumperEnd;
//hill textures
GameTexture hill;
//GUI textures
GameTexture arrow;
//Text textures
GameTexture text_wind;
GameTexture text_gate;
GameTexture text_speed;
GameTexture text_distance;
GameTexture text_player;
GameTexture text_playerName;

//================= AUXILLARY FUNCTIONS ================================

std::random_device rd;
std::mt19937 mt(rd());

//returns integer number in given interval
int randBetween(int a, int b)
{
	std::uniform_int_distribution<int> dist(a,b);
	int num = dist(rd);
	return num;
}

//cuts useless zeros at the end of float converted to string
string trimzero(string text)
{
	string newtex = "";
	for (int i = 0; i < 5; i++)
	{
		newtex += text[i];
	}
	return newtex;
}


//=============== TEXTURE OBJECT METHODS AND PROPERTIES ===================

//main color for text
SDL_Color textColor = { 0, 0, 0 };

//creates font size with proportion to used screen width. Default constant is 72, smaller it is - the bigger is the font. Limits are fixed.
int getFontSizeRelativeToResolutionByConstant(float fontconst = 72.0)
{
	int number = static_cast<int>(ceil(static_cast<float>(SCREEN_WIDTH) / fontconst));
	if (number < 16) number = 16;
	return number;
}

//font size basing on screen width
int fontSize = getFontSizeRelativeToResolutionByConstant();

//loads texture from file
bool GameTexture::loadFromFile(std::string path)
{
	free();
	SDL_Texture* newTexture = NULL;
	//load image to surface first
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		//SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface
		newTexture = SDL_CreateTextureFromSurface(gameRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image width and height using its surface
			width = loadedSurface->w;
			height = loadedSurface->h;
		}

		//Delete surface
		SDL_FreeSurface(loadedSurface);
	}

	hTexture = newTexture;
	if (hTexture != NULL) return true;
	else return false;
}

/*
Renders texture on screen
x,y - coordinates of left top corner (int)
clip - part to be cut from texture (Rect)
angle - angle of rotation of texture (double)
center - center of rotation
flip - type of texture flipping
*/
void GameTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rectangle for rendering
	SDL_Rect rendRect = { x,y,width,height };

	//clip the texture if required
	if (clip != NULL)
	{
		rendRect.w = clip->w;
		rendRect.h = clip->h;
	}

	//render to screen
	SDL_RenderCopyEx(gameRenderer, hTexture, clip, &rendRect, angle, center, flip);
}

//loads text texture to texture object with given properties
bool GameTexture::loadFromRenderedText(std::string textureText, SDL_Color txc, int size)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	TTF_Font* gFont = TTF_OpenFont("fonts/Titillium-Bold.otf", size);
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), txc);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		//Create texture from surface pixels
		hTexture = SDL_CreateTextureFromSurface(gameRenderer, textSurface);
		if (hTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			width = textSurface->w;
			height = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	//Return success
	return hTexture != NULL;
}

//====================== LOADING / DISLOADING COMPONENTS FUNCTIONS ===========================

//loads all SDL modules and their components
bool init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering is not enabled!");
		}

		//Create window of the game
		gameWindow = SDL_CreateWindow("ski jumping by tuvrai", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gameWindow == NULL)
		{
			printf("Failed to create window of the game! SDL Error: %s\n", SDL_GetError());
			return false;
		}
		else
		{
			//Create texture renderer
			gameRenderer = SDL_CreateRenderer(gameWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gameRenderer == NULL)
			{
				printf("Failed to run the renderer! SDL Error: %s\n", SDL_GetError());
				return false;
			}
			else
			{
				//Set default renderer color
				SDL_SetRenderDrawColor(gameRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Enable PNG graphics
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					return false;
				} 
				if (TTF_Init() == -1)
				{
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					return false;
				}
			}
		}
	}

	return true;
}

//loads media like images, sounds etc
bool loadMedia()
{
	//jumper texture before landing
	if (!jumperTex.loadFromFile("img/jumper_start.png"))
	{
		return false;
	}
	//jumper texture after landing
	if (!jumperEnd.loadFromFile("img/jumper_land.png"))
	{
		return false;
	}
	//hill
	if (!hill.loadFromFile("img/hill.png"))
	{
		return false;
	}
	//arrow
	if (!arrow.loadFromFile("img/arrow.png"))
	{
		return false;
	}
	return true;
}

//desstroys objects and SDL components
void close()
{
	//Free loaded image textures
	jumperTex.free();
	jumperEnd.free();
	hill.free();
	arrow.free();
	//Free text textures
	text_wind.free();
	text_gate.free();
	text_speed.free();
	text_distance.free();
	text_player.free();
	text_playerName.free();
	//Free menu textures
	menu_title.free();
	 menu_opt1.free();
	 menu_opt2.free();
	 menu_opt3.free();
	 menu_opt4.free();
	 caption.free();
	//Destroy renderer
	SDL_DestroyRenderer(gameRenderer);
	gameRenderer = NULL;
	//Destroy window
	SDL_DestroyWindow(gameWindow);
	gameWindow = NULL;

	//Quit SDL libraries
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

//=============================== TAKE-OFF STARTING GATES POSITIONS ========================

//number of gates
const int gatenum = 8;

//gates positions
pos gates[gatenum] = {
	{1164,977},
	{1132,945},
	{1100,913},
	{1068,881},
	{1036,849},
	{1004,817},
	{972,785},
	{940,753}
};

//starting default gate
int defGate = 2;

//STARTING COORDINATES
pos hillA = gates[defGate];

//font colors
SDL_Color color_black = { 0,0,0 };
SDL_Color color_white = { 255, 255, 255 };
SDL_Color color_green = { 0 ,255 , 0 };

//=========================SLIDING CALIBRATION========================================

//Sum of anchor points of the slide
const int slide_count = 270;

//x-coordinates to given hill anchor
int slideX[slide_count] = { 979, 983, 986, 990, 993, 997, 1000, 1004, 1007, 1011, 1014, 1018, 1021, 1025, 1029, 1032, 1036, 1039, 1043, 1046, 1050, 1053, 1057, 1060, 1064, 1067, 1071, 1074, 1078, 1082, 1085, 1089, 1092, 1096, 1099, 1103, 1106, 1110, 1113, 1117, 1120, 1124, 1128, 1131, 1135, 1138, 1142, 1145, 1149, 1152, 1156, 1159, 1163, 1166, 1170, 1173, 1177, 1181, 1184, 1188, 1191, 1195, 1198, 1202, 1205, 1209, 1212, 1216, 1220, 1223, 1227, 1230, 1234, 1237, 1241, 1245, 1248, 1252, 1255, 1259, 1263, 1266, 1270, 1274, 1277, 1281, 1285, 1288, 1292, 1296, 1299, 1303, 1307, 1310, 1314, 1318, 1321, 1325, 1329, 1332, 1336, 1340, 1343, 1347, 1351, 1354, 1358, 1362, 1365, 1369, 1372, 1376, 1380, 1383, 1387, 1391, 1394, 1398, 1402, 1405, 1409, 1413, 1416, 1420, 1424, 1427, 1431, 1435, 1438, 1442, 1446, 1449, 1453, 1457, 1460, 1464, 1469, 1472, 1476, 1480, 1483, 1487, 1491, 1495, 1498, 1502, 1506, 1510, 1514, 1517, 1521, 1525, 1529, 1532, 1536, 1540, 1544, 1547, 1551, 1555, 1559, 1563, 1567, 1571, 1575, 1579, 1583, 1587, 1591, 1595, 1599, 1603, 1607, 1612, 1616, 1620, 1624, 1629, 1633, 1637, 1641, 1645, 1649, 1654, 1658, 1663, 1667, 1671, 1676, 1680, 1684, 1689, 1693, 1697, 1702, 1707, 1712, 1716, 1721, 1725, 1730, 1734, 1740, 1744, 1749, 1753, 1758, 1763, 1767, 1772, 1777, 1781, 1787, 1792, 1796, 1801, 1806, 1811, 1815, 1820, 1825, 1830, 1834, 1840, 1845, 1850, 1855, 1860, 1864, 1869, 1874, 1879, 1884, 1889, 1894, 1898, 1903, 1908, 1913, 1918, 1923, 1928, 1933, 1938, 1942, 1948, 1953, 1958, 1963, 1968, 1973, 1978, 1983, 1988, 1992, 1997, 2002, 2007, 2012, 2017, 2022, 2027, 2032, 2037, 2042, 2047, 2052, 2057, 2062, 2067 };

//y-coordinates to given hill anchor
int slideY[slide_count] = { 794, 798, 801, 805, 808, 812, 815, 819, 822, 826, 829, 833, 836, 840, 844, 847, 851, 854, 858, 861, 865, 868, 872, 875, 879, 882, 886, 889, 893, 897, 900, 904, 907, 911, 914, 918, 921, 925, 928, 932, 935, 939, 943, 946, 950, 953, 957, 960, 964, 967, 971, 974, 978, 981, 985, 988, 992, 996, 999, 1003, 1006, 1010, 1013, 1017, 1020, 1024, 1027, 1031, 1034, 1038, 1041, 1045, 1048, 1052, 1055, 1059, 1062, 1066, 1069, 1073, 1076, 1079, 1083, 1086, 1090, 1093, 1096, 1100, 1103, 1107, 1110, 1113, 1117, 1120, 1124, 1127, 1131, 1134, 1137, 1141, 1144, 1148, 1151, 1154, 1158, 1161, 1165, 1168, 1171, 1175, 1178, 1182, 1185, 1188, 1192, 1195, 1199, 1202, 1206, 1209, 1212, 1216, 1219, 1223, 1226, 1229, 1233, 1236, 1240, 1243, 1246, 1250, 1253, 1257, 1260, 1263, 1267, 1270, 1274, 1277, 1280, 1284, 1287, 1290, 1294, 1297, 1300, 1303, 1307, 1310, 1313, 1317, 1320, 1323, 1326, 1330, 1333, 1336, 1339, 1343, 1346, 1349, 1352, 1355, 1359, 1362, 1365, 1368, 1371, 1374, 1377, 1380, 1383, 1385, 1388, 1391, 1393, 1396, 1399, 1402, 1405, 1407, 1410, 1413, 1416, 1418, 1421, 1424, 1426, 1429, 1431, 1434, 1436, 1439, 1441, 1442, 1445, 1447, 1449, 1451, 1453, 1456, 1458, 1460, 1462, 1463, 1465, 1467, 1468, 1470, 1472, 1474, 1476, 1477, 1479, 1481, 1482, 1484, 1485, 1486, 1487, 1489, 1490, 1492, 1493, 1494, 1496, 1497, 1498, 1499, 1500, 1502, 1503, 1504, 1505, 1506, 1507, 1508, 1508, 1509, 1510, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519, 1519, 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1529, 1530, 1531, 1532, 1533, 1533, 1534, 1535, 1536, 1536 };

//angles to given hill anchor
int slideA[slide_count] = { 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 42, 42, 42, 42, 42, 42, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 40, 40, 40, 39, 39, 39, 38, 38, 38, 38, 38, 38, 36, 36, 36, 35, 35, 35, 34, 34, 34, 34, 34, 34, 33, 33, 33, 31, 31, 31, 31, 30, 30, 30, 29, 29, 29, 27, 27, 27, 25, 25, 25, 25, 24, 24, 24, 22, 22, 22, 22, 21, 21, 21, 19, 19, 19, 19, 18, 18, 18, 17, 17, 17, 17, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 5 };

//slide anchors done so far
int slidone = 0;


//========================HILL CALIBRATION==============================

//Sum of anchor points of the hill
const int hill_count = 571;

//Angles to given hill anchor
int a_hill[hill_count] = { -7, -7, -7, -6, -6, -6, -6, -5, -5, -5, -5, -5, -5, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 1, 1, 2, 2, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 10, 10, 11, 11, 12, 12, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 18, 18, 18, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 24, 23, 23, 23, 22, 22, 22, 22, 21, 21, 21, 20, 20, 19, 17, 16, 15, 15, 14, 13, 11, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 7, 7, 6, 6, 6, 6, 6, 7, 7, 7, 7, 6, 6, 6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 0, 0, -1, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -5, -5, -5, -5, -5, -5, -5, -5, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -5, -5, -5, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4 };

//x-coordinates to given hill anchor
int x_hill[hill_count] = { 2054, 2059, 2064, 2069, 2074, 2079, 2084, 2089, 2094, 2099, 2104, 2109, 2114, 2119, 2124, 2129, 2134, 2139, 2144, 2149, 2154, 2159, 2164, 2169, 2174, 2179, 2184, 2189, 2194, 2199, 2204, 2209, 2214, 2219, 2224, 2229, 2234, 2239, 2244, 2249, 2254, 2259, 2264, 2269, 2274, 2279, 2284, 2289, 2294, 2299, 2304, 2309, 2313, 2318, 2323, 2328, 2333, 2338, 2343, 2348, 2353, 2358, 2362, 2367, 2372, 2377, 2382, 2387, 2392, 2396, 2401, 2406, 2411, 2416, 2420, 2425, 2430, 2435, 2440, 2444, 2449, 2454, 2458, 2463, 2468, 2473, 2477, 2482, 2487, 2491, 2496, 2501, 2505, 2510, 2515, 2520, 2524, 2529, 2534, 2538, 2543, 2548, 2552, 2557, 2561, 2566, 2571, 2575, 2580, 2584, 2589, 2594, 2598, 2603, 2607, 2612, 2616, 2621, 2626, 2630, 2635, 2639, 2644, 2648, 2653, 2657, 2662, 2666, 2671, 2675, 2680, 2684, 2689, 2693, 2698, 2703, 2707, 2712, 2716, 2721, 2725, 2730, 2734, 2738, 2743, 2747, 2752, 2756, 2760, 2765, 2769, 2774, 2778, 2782, 2787, 2791, 2795, 2800, 2804, 2809, 2813, 2818, 2822, 2826, 2831, 2835, 2840, 2844, 2848, 2853, 2857, 2862, 2866, 2870, 2875, 2879, 2884, 2888, 2893, 2897, 2901, 2906, 2910, 2915, 2919, 2923, 2928, 2932, 2937, 2941, 2946, 2950, 2954, 2959, 2963, 2968, 2972, 2976, 2981, 2985, 2989, 2994, 2998, 3003, 3007, 3011, 3016, 3020, 3024, 3029, 3033, 3037, 3041, 3046, 3050, 3054, 3059, 3063, 3067, 3072, 3076, 3080, 3085, 3089, 3093, 3098, 3102, 3106, 3111, 3115, 3119, 3124, 3128, 3132, 3137, 3141, 3145, 3150, 3154, 3158, 3163, 3167, 3171, 3176, 3180, 3184, 3188, 3193, 3197, 3201, 3206, 3210, 3214, 3218, 3223, 3227, 3231, 3236, 3240, 3244, 3248, 3253, 3257, 3261, 3266, 3270, 3274, 3278, 3283, 3287, 3291, 3296, 3300, 3304, 3308, 3313, 3317, 3321, 3326, 3330, 3334, 3338, 3343, 3347, 3351, 3355, 3360, 3364, 3368, 3372, 3376, 3381, 3385, 3389, 3393, 3398, 3402, 3406, 3410, 3414, 3418, 3423, 3427, 3431, 3435, 3439, 3443, 3447, 3452, 3456, 3460, 3464, 3468, 3472, 3476, 3481, 3485, 3489, 3493, 3497, 3502, 3506, 3510, 3514, 3518, 3522, 3527, 3531, 3535, 3539, 3543, 3548, 3552, 3556, 3560, 3564, 3569, 3573, 3577, 3581, 3586, 3590, 3594, 3598, 3603, 3607, 3611, 3615, 3620, 3624, 3628, 3633, 3637, 3641, 3645, 3650, 3654, 3658, 3663, 3667, 3671, 3676, 3680, 3684, 3689, 3693, 3698, 3702, 3706, 3711, 3715, 3719, 3724, 3728, 3733, 3737, 3741, 3746, 3750, 3755, 3759, 3763, 3768, 3772, 3777, 3781, 3786, 3790, 3794, 3799, 3803, 3808, 3812, 3817, 3821, 3825, 3830, 3835, 3839, 3844, 3848, 3853, 3857, 3862, 3867, 3871, 3876, 3881, 3885, 3890, 3894, 3899, 3904, 3909, 3913, 3918, 3923, 3928, 3933, 3937, 3942, 3947, 3952, 3957, 3962, 3967, 3972, 3977, 3982, 3987, 3992, 3996, 4001, 4006, 4011, 4016, 4021, 4026, 4031, 4036, 4041, 4046, 4051, 4056, 4061, 4066, 4071, 4076, 4081, 4086, 4091, 4096, 4101, 4106, 4111, 4116, 4121, 4126, 4130, 4135, 4140, 4145, 4150, 4155, 4160, 4165, 4170, 4175, 4180, 4185, 4190, 4195, 4200, 4205, 4210, 4215, 4220, 4225, 4230, 4235, 4240, 4245, 4250, 4255, 4260, 4265, 4270, 4275, 4280, 4285, 4290, 4295, 4300, 4305, 4310, 4315, 4320, 4325, 4330, 4335, 4340, 4345, 4350, 4355, 4360, 4365, 4370, 4375, 4380, 4385, 4390, 4395, 4400, 4405, 4410, 4415, 4420, 4425, 4430, 4435, 4440, 4445, 4450, 4455, 4460, 4465, 4470, 4475, 4480, 4485, 4490, 4495, 4500, 4505, 4510, 4515, 4519, 4524, 4529, 4534, 4539, 4544, 4549, 4554, 4559, 4564, 4569, 4574, 4579, 4584, 4589, 4594, 4599, 4604, 4609, 4614, 4619, 4624, 4629, 4634, 4639, 4644, 4649, 4654, 4659, 4664, 4669, 4674, 4679, 4684, 4689 };

//y-coordinates to given hill anchor
int y_hill[hill_count] = { 1631, 1630, 1630, 1629, 1629, 1628, 1628, 1627, 1627, 1626, 1626, 1625, 1625, 1625, 1624, 1624, 1624, 1623, 1623, 1623, 1622, 1622, 1622, 1621, 1621, 1621, 1621, 1621, 1620, 1620, 1620, 1620, 1621, 1621, 1621, 1621, 1621, 1622, 1622, 1623, 1623, 1623, 1624, 1624, 1624, 1625, 1625, 1626, 1627, 1627, 1628, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, 1643, 1644, 1645, 1646, 1647, 1649, 1650, 1651, 1653, 1654, 1655, 1657, 1658, 1660, 1661, 1663, 1665, 1666, 1668, 1670, 1671, 1673, 1675, 1676, 1678, 1680, 1682, 1683, 1685, 1687, 1688, 1690, 1692, 1694, 1695, 1697, 1699, 1701, 1703, 1705, 1707, 1709, 1711, 1713, 1715, 1716, 1718, 1720, 1722, 1724, 1726, 1728, 1730, 1733, 1735, 1737, 1739, 1741, 1743, 1745, 1747, 1749, 1752, 1754, 1756, 1758, 1760, 1762, 1764, 1766, 1769, 1771, 1773, 1775, 1777, 1779, 1781, 1784, 1786, 1788, 1791, 1793, 1795, 1798, 1800, 1803, 1805, 1807, 1810, 1812, 1815, 1817, 1820, 1822, 1824, 1827, 1829, 1831, 1834, 1836, 1838, 1841, 1843, 1845, 1848, 1850, 1852, 1855, 1857, 1859, 1862, 1864, 1866, 1869, 1871, 1874, 1876, 1878, 1881, 1883, 1885, 1888, 1890, 1892, 1895, 1897, 1899, 1902, 1904, 1906, 1909, 1911, 1914, 1916, 1918, 1921, 1923, 1926, 1928, 1931, 1933, 1936, 1938, 1941, 1943, 1946, 1948, 1951, 1953, 1956, 1958, 1961, 1963, 1966, 1968, 1971, 1973, 1976, 1978, 1981, 1983, 1986, 1988, 1991, 1993, 1996, 1998, 2001, 2003, 2006, 2008, 2011, 2013, 2016, 2018, 2021, 2023, 2026, 2028, 2031, 2033, 2036, 2039, 2041, 2044, 2046, 2049, 2052, 2054, 2057, 2059, 2062, 2064, 2067, 2070, 2072, 2075, 2077, 2080, 2082, 2085, 2088, 2090, 2093, 2095, 2098, 2100, 2103, 2106, 2108, 2111, 2113, 2116, 2118, 2121, 2124, 2126, 2129, 2132, 2134, 2137, 2140, 2142, 2145, 2147, 2150, 2153, 2155, 2158, 2161, 2164, 2166, 2169, 2172, 2174, 2177, 2180, 2183, 2186, 2188, 2191, 2194, 2197, 2200, 2202, 2205, 2208, 2211, 2214, 2216, 2219, 2222, 2225, 2227, 2230, 2233, 2236, 2238, 2241, 2244, 2247, 2249, 2252, 2255, 2257, 2260, 2263, 2266, 2268, 2271, 2274, 2276, 2279, 2282, 2284, 2287, 2290, 2292, 2295, 2298, 2300, 2303, 2306, 2308, 2311, 2313, 2316, 2318, 2321, 2324, 2326, 2329, 2331, 2334, 2336, 2339, 2341, 2344, 2346, 2348, 2351, 2353, 2356, 2358, 2361, 2363, 2365, 2368, 2370, 2373, 2375, 2378, 2380, 2382, 2385, 2387, 2389, 2392, 2394, 2396, 2399, 2401, 2403, 2406, 2408, 2410, 2413, 2415, 2417, 2420, 2422, 2424, 2426, 2428, 2430, 2432, 2434, 2436, 2438, 2440, 2442, 2444, 2446, 2448, 2450, 2452, 2453, 2455, 2457, 2459, 2460, 2462, 2463, 2464, 2466, 2467, 2468, 2469, 2470, 2471, 2472, 2472, 2473, 2474, 2475, 2476, 2476, 2477, 2478, 2479, 2479, 2480, 2481, 2482, 2482, 2483, 2484, 2484, 2485, 2485, 2486, 2486, 2487, 2487, 2488, 2489, 2489, 2490, 2490, 2491, 2491, 2492, 2492, 2493, 2493, 2494, 2494, 2495, 2495, 2496, 2496, 2497, 2497, 2498, 2498, 2499, 2499, 2499, 2500, 2500, 2500, 2500, 2501, 2501, 2501, 2501, 2502, 2502, 2502, 2503, 2503, 2503, 2503, 2504, 2504, 2504, 2504, 2505, 2505, 2505, 2505, 2505, 2505, 2505, 2505, 2505, 2504, 2504, 2504, 2503, 2503, 2503, 2502, 2502, 2502, 2501, 2501, 2501, 2500, 2500, 2499, 2499, 2499, 2498, 2498, 2498, 2497, 2497, 2497, 2496, 2496, 2496, 2495, 2495, 2494, 2494, 2493, 2493, 2493, 2492, 2492, 2491, 2491, 2490, 2490, 2489, 2488, 2488, 2487, 2487, 2486, 2486, 2485, 2485, 2484, 2484, 2483, 2483, 2483, 2482, 2482, 2482, 2481, 2481, 2481, 2480, 2480, 2480, 2479, 2479, 2478, 2478, 2478, 2477, 2477, 2477, 2476 };

//hill anchores done so far
int hildone = 0;

//additional positions done counter
int pdone = 0;

//==================== FUNCTIONS AND METHODS COORDINATING PROCESS OF JUMP ====================

//Standard default jumper
Jumper jumper;

//returns distance in meters for given coordinates to the 0.5m precision (Default) or exact
double getDistanceJumped(int x, int y, bool exact = false)
{
	int takeoffX = slideX[slide_count - 1]-30;
	int takeoffY = slideY[slide_count - 1]-19;
	double dist = sqrt(pow((takeoffX - x), 2) + pow((takeoffY - y), 2));
	double dist_m = dist / 15;

	if (!exact)
	{
		int dist_round = static_cast<int>(dist_m);
		float diff = dist_m - dist_round;
		if (diff > 0.25 && diff <= 0.75)
		{
			dist_m = dist_round + 0.5;
		}
		else dist_m = dist_round;
	}

	return dist_m;
}

//checks whether the jumper has touched the ground. If so, it reacts on it.
void Jumper::checkLand(int ex,int ey)
{
	if (this->x < 2069)
	{
		if (slidone < 0) slidone = 0;
		while (ex > slideX[slidone] && slidone < slide_count)
		{
			slidone++;
		}
		if (slidone >= slide_count) slidone = slide_count-1;
		hillangle = slideA[slidone];
		if (slidone < slide_count && slideY[slidone] <= ey)
		{
			crash = true;
			flying = false;
			jumper.setAngle(slideA[slidone]);
			pos edge = getSkiEdge();
			int offx = edge.x - x;
			int offy = edge.y - y;
			jumper.setX(slideX[slidone] - tx - tx);
			jumper.setY(slideY[slidone] - ty - ty);
			//jumper.changeAngle(-1);
			jumper.moveY(-12);
			slidone -= 50;
			checkSlide();
			jumperTex.loadFromFile("img/jumper_crash.png");
			jumperEnd.loadFromFile("img/jumper_crash.png");
		}
	}
	pdone -=10;
	if (pdone < 0) pdone = 0;
	while ( ex > x_hill[pdone] && pdone < hill_count)
	{
		pdone++;
	}
	hillangle = a_hill[pdone];
	if (pdone < hill_count && y_hill[pdone] <= ey) 
	{ 
		landed = true;
		flying = false;
		int offx = ex - x;
		int offy = ey - y;
		jumper.setX(x_hill[pdone] - offx);
		jumper.setY(y_hill[pdone] - offy + 10);

		jumper.distance = getDistanceJumped(x, y, false);
		createDistanceText(jumper.distance);
		
	}
}

//coordinates jumper's position when sliding of the hill
void Jumper::checkHill()
{
	hildone -= 10;
	int len = x + tx + tx;
	if (hildone < 0) hildone = 0;
	while (hildone < hill_count && len > x_hill[hildone] && hildone < hill_count)
	{
		hildone++;
	}
	if (hildone < hill_count) jumper.setAngle(a_hill[hildone]);
	if (len > x_hill[hill_count-1]) {
		vconst = 0;
		started = false;
	}
	if (jumper.y < y_hill[hildone] - 20) jumper.y += 1;
}

//coordinates jumper's position when sliding of the ramp
void Jumper::checkSlide()
{
	if (slidone < 0) slidone = 0;
	int len = x + tx + tx;
	while (slidone < slide_count && len > slideX[slidone] && slidone < slide_count)
	{
		slidone++;
	}
	if (slidone < slide_count) this->setAngle(slideA[slidone]);

}

//returns jumper's closest point to the ground
pos Jumper::getSkiEdge()
{
	double angleRad = angle * M_PI / 180.0;
	double param = double(ty) / double(tx);
	double natan = atan(param);
	pos p;
	if (angle < 0) natan = -natan;
	double cosx = cos(angleRad + natan);
	double sinx = 0;
	sinx = sin(angleRad + natan);
	//if (angle == hillangle) { p.x = x; p.y = y + ty; }
	if (angle > hillangle)
	{
		p.x = x + int(len * cosx);
		p.y = y + int(len * sinx);
	}
	else if (angle <= hillangle)
	{
		if (angle >= 0)
		{
			cosx = cos(angleRad - natan);
			sinx = sin(angleRad - natan);
		}
		p.x = x - int(len * cosx);
		p.y = y - int(len * sinx);
	}
	p.x += tx;
	p.y += ty;
	return p;
}

//puts jumper in starting position at default gate enabling to jump another time
void jumper_reset()
{
	jumper.vconst = 0;
	jumper.landed = false;
	jumper.crash = false;
	jumper.flying = false;
	jumper.started = false;
	jumper.setX(gates[defGate].x);
	jumper.setY(gates[defGate].y);
	setGate(defGate);
	jumper.setAngle(45);
	jumperTex.loadFromFile("img/jumper_start.png");
	jumperEnd.loadFromFile("img/jumper_land.png");
	hildone = 0;
	slidone = 0;
}

//randomizes wind in one direction
float randwind()
{
	return  randBetween(-20, 20) / 10.f;
}

//allows to set jumper's gate before the jump
void setGate(int gate)
{
	string text;
	if (gate >= 0 && gate < gatenum && !jumper.started && !jumper.landed)
	{
		text = "Gate: " + to_string(gate + 1);
		text_gate.loadFromRenderedText(text, color_black, fontSize);
		jumper.setX(gates[gate].x);
		jumper.setY(gates[gate].y);
	}
}

//====================== TEXT AND LAYOUT FUNCTIONS =================================

//creates text showing take-off speed after taking off
void createSpeedText(float speed)
{
	speed = ceil(speed * 100.0) / 100.0;
	string text = "Speed: ";
	text+= trimzero(to_string(speed));
	text_speed.loadFromRenderedText(text, color_black, fontSize);
}

//creates information about distance jumped after landing
void createDistanceText(float distance)
{
	string text = "Distance: " + trimzero(to_string(distance))+ "m";
	text_distance.loadFromRenderedText(text, color_black, fontSize);
}

//creates player name information before the jump
void createPlayerName(string name)
{
	text_playerName.loadFromRenderedText(name, color_black, fontSize);
}

//check whether the user entered name that is allowed
bool checkName(string name)
{
	int len = name.size();
	for (int i = 0; i < len; i++)
	{
		if ((name[i] >= 65 && name[i] <= 90) || (name[i] >= 97 && name[i] <= 122) || (name[i] >= 48 && name[i] <= 57)) continue;
		else return false;
	}
	if (name != "") return true;
	else return false;
}

//parses players from array into string which will be a row in the list
string parsePlayerArrayToText(string arr[], int start, int end, int maxsize)
{
	string parsetext = "";
	for (int i = start;(i <= end && i < maxsize);i++)
	{
		parsetext += "[" + to_string(i+1) + "]" + arr[i] + "\n";
	}
	return parsetext;
}

//count players if tournament
int countPlayers()
{
	int count = 0;
	for (int i = 0; i < 30; i++)
	{
		if (players[i] != "") count++;
		else return count;
	}
	return count;
}


//=========MAIN

//process of the actual ski jumping game. argument "true" switches mode to tournament.
void run(bool tour = false)
{
			
			//STARTING JUMPER BASE SETTINGS

			jumper.setX(hillA.x);
			jumper.setY(hillA.y);
			jumper.setAngle(45);
			//point of jumper's closest point to ground
			pos edge;
			//jumper speed modifier
			jumper.vconst = 0;

			//JUMPER'S TEXTURE DIMENSIONS, respectively: half-width, half-height, diagonal

			jumper.tx = jumperTex.getWidth() / 2;
			jumper.ty = jumperTex.getHeight() / 2;
			jumper.len = sqrt(double(jumper.tx * 2) * double(jumper.tx * 2) / 4 + double(jumper.ty * 2) * double(jumper.ty * 2) / 4);

			//key states flags

			bool isL = false;
			bool isR = false;

			//HILL CLIP - ACTING AS A 'CAMERA' VIEW, CUTTING PART OF HILL IMAGE (FIXED SIZE), CENTERING THE JUMPER

			SDL_Rect hillClip;
			hillClip.w = SCREEN_WIDTH;
			hillClip.h = SCREEN_HEIGHT;

			//FRAME COUNTER
			int framecount = 0;


			//ski edge x offset from jumper.x
			int offx = 0;
			//ski edge y offset from jumper.y
			int offy = 0;

			//bigger the value, the stronger and longer the take off will affect flight
			int jump_buffer = 0;

			//speed while taking off
			double jump_speed = 0;

			//WIND GENERATING AND PUTTING TO TEXT TEXTURE

			Wind wind = Wind(randwind(), randwind());
			float value = ceil(wind.getForce() * 100.0) / 100.0;
			string text = "Wind: " + trimzero(to_string(value));
			text_wind.loadFromRenderedText(text, color_black, fontSize);

			//GATE HANDLING

			text = "Gate: " + to_string(defGate + 1);
			text_gate.loadFromRenderedText(text, color_black, fontSize);
			//gate used, changed by page-up / page-down
			int gate_set = defGate;

			//PLAYER NAME

			//Indicates player id who will jump next (tournament)
			int playerid = 0;

			int playeramount = 0;

			bool endTour = false;

			//Player's name who is going to jump next
			string playerNow;

			text = "Player: ";
			text_player.loadFromRenderedText(text, color_black, fontSize);
			//player who jumps
			if (!tour) playerNow = "Training";
			else
			{
				playeramount = countPlayers();
				if (playeramount == 0 || playeramount > 30) endTour = true;
				playerNow = players[playerid];
			}
			createPlayerName(playerNow);

			bool quit = false;
			if (endTour) quit = true;
			jumper_reset();


			//loop keeping process alive until it's exited by window event
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//Quitting event
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}

					//Keyboard events
					else
					{
						//Key down events
						if (e.type == SDL_KEYDOWN)
						{
							switch (e.key.keysym.sym)
							{
								//ANGLE CHANGES COUNTERCLOCKWISE
							case SDLK_LEFT:
								isL = true;
								break;
								//ANGLE CHANGES CLOCKWISE
							case SDLK_RIGHT:
								isR = true;
								break;
								//START/TAKEOFF
							case SDLK_SPACE:
								if (!jumper.started) jumper.started = true;
								else if (!jumper.flying && !jumper.crash)
								{
									jumper.changeAngle(-6);
									jumper.flying = true;
									jumperTex.loadFromFile("img/jumper_fly.png");
									jumper.vy = 2;
									jump_buffer = 28;
									jump_speed = jumper.vconst;
									createSpeedText(jump_speed);
								}
								break;
								//GATE CHANGE TO HIGHER
							case SDLK_PAGEUP:
								setGate(++gate_set);
								break;
								//GATE CHANGE TO LOWER
							case SDLK_PAGEDOWN:
								setGate(--gate_set);
								break;
							case SDLK_c:
								break;
								//JUMP RESTART
							case SDLK_r:
								if (!jumper.flying && jumper.started)
								{
									gate_set = defGate;
									jumper_reset();
									if (tour)
									{
										distances[playerid] = jumper.distance;
										jumper.distance = 0;
										playerid++;
										if (playerid >= playeramount)
										{
											endTour = true;
											quit = true;
										}
										else
										{
											playerNow = players[playerid];
											createPlayerName(playerNow);
										}
									}
								}
									break;
								
								//END PROCESS OF JUMPING
							case SDLK_ESCAPE:
								quit = true;
								break;
							}
						}

						//Key up events
						if (e.type == SDL_KEYUP)
						{
							switch (e.key.keysym.sym)
							{
								//STOP CHANGING ANGLE COUNTERCLOCKWISE
							case SDLK_LEFT:
								isL = false;
								break;
								//STOP CHANGING ANGLE CLOCKWISE
							case SDLK_RIGHT:
								isR = false;
								break;
							}
						}
					}

				}

				if (jumper.started)
				{
					if (!jumper.flying && !jumper.landed && !jumper.crash) //ramp movement
					{
						//jumper's takeoff acceleration
						jumper.vconst += 0.28 / 1.5f;
						jumper.vx = jumper.vconst * cos(jumper.getAngle() * M_PI / 180);
						jumper.vy = jumper.vconst * sin(jumper.getAngle() * M_PI / 180);

						jumper.moveX(jumper.vx);
						jumper.moveY(jumper.vy);
					}
					else if (jumper.crash && jumper.getX() < 2069) //crash on ramp due to jumping to early
					{
						jumper.vconst += 0.1 / 1.5f;
						if (jumper.vconst > 11) jumper.vconst = 11;
						jumper.vx = jumper.vconst * cos(jumper.getAngle() * M_PI / 180);
						jumper.vy = jumper.vconst * sin(jumper.getAngle() * M_PI / 180);

						jumper.moveX(jumper.vx);
						jumper.moveY(jumper.vy);
					}
					else if (jumper.crash && !jumper.landed) //falling down after crashing on ramp
					{
						jumper.flying = true;
						jumper.vx -= 0.05;
						jumper.moveX(jumper.vx);
						jumper.moveY(5);
					}
					else if (jumper.flying && !jumper.landed) //flying after take-off
					{
						//take-off modifier
						if (jump_buffer > 0)
						{
							jumper.vconst += jump_buffer * (jump_speed / 1000.f);
							jumper.moveY(-jump_buffer * (jump_speed / 200.f));
							jump_buffer--;
						}
						//air resitance
						jumper.vconst -= 0.18;
						//jumper's x-speed depending of angle of flight
						jumper.vx = jumper.vconst * cos((jumper.getAngle() + 15) * M_PI / 180);
						//x-wind modifier
						jumper.vx += wind.getX() * 0.17;
						//setting minimum value of x-velocity
						if (jumper.vx < 0.4) jumper.vx = 0.4;
						//moving the jumper according to his speed (x)
						jumper.moveX(jumper.vx);
						//gravitational acceleration
						jumper.vy += 0.23;
						//y-wind modifier
						jumper.vy -= wind.getY() * 0.012;
						//moving the jumper according to his speed (y)
						jumper.moveY(jumper.vy);
						//jumper's angle change due to the inbalance
						jumper.changeAngle(randBetween(-2, 2));
					}
					else if (jumper.landed) //landing on the hill's snow
					{
						jumper.vconst += 0.04 / 1.5f;
						jumper.vx = jumper.vconst * cos(jumper.getAngle() * M_PI / 180);
						jumper.vy = jumper.vconst * sin(jumper.getAngle() * M_PI / 180);
						jumper.moveX(jumper.vx);
						jumper.moveY(jumper.vy);
					}

					//if on the ramp, coordinate ramp movement
					if (!jumper.flying && slidone < slide_count) jumper.checkSlide();
					else if (!jumper.flying && !jumper.landed && !jumper.crash) //if ramp eneded, start flying
					{
						jumper.flying = true;
						jumperTex.loadFromFile("img/jumper_fly.png");
					}
					//coordinates jumper's movement after flying had ended due to the crash or landing
					if (jumper.getX() < 2069 && jumper.landed) {
						jumper.checkSlide();
					}
					else if (jumper.landed && hildone < hill_count) jumper.checkHill();

					if (jumper.flying)
					{
						//keyboard input
						if (isR) jumper.changeAngle(3);
						if (isL) jumper.changeAngle(-3);
						//While flying, get jumper's ski point closest to the ground
						edge = jumper.getSkiEdge();
						jumper.checkLand(edge.x, edge.y);
						offx = edge.x - round(jumper.getX());
						offy = edge.y - round(jumper.getY());
					}
				}

				//Clear screen
				SDL_SetRenderDrawColor(gameRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gameRenderer);

				//generating view of the hill, basing on jumper's position
				hillClip.x = jumper.getX() - SCREEN_WIDTH / 2;
				hillClip.y = jumper.getY() - SCREEN_HEIGHT / 2;
				hill.render(0, 0, &hillClip, NULL, NULL, SDL_FLIP_NONE);

				//RENDERING
				if (!jumper.landed) jumperTex.render(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, NULL, jumper.getAngle(), NULL, SDL_FLIP_NONE);
				else jumperEnd.render(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, NULL, jumper.getAngle(), NULL, SDL_FLIP_NONE);
				//if (jumper.flying) DrawCircle(gameRenderer, SCREEN_WIDTH / 2 + offx, SCREEN_HEIGHT / 2 + offy, 2);

				arrow.render(SCREEN_WIDTH - arrow.getWidth() - 10, arrow.getHeight(), NULL, wind.getAngle(), NULL, SDL_FLIP_NONE);
				text_wind.render(SCREEN_WIDTH - text_wind.getWidth() - 10, arrow.getWidth() + arrow.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				text_gate.render(SCREEN_WIDTH - text_gate.getWidth() - 10, arrow.getWidth() + arrow.getHeight() + text_wind.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				text_player.render(10, SCREEN_HEIGHT - text_player.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				text_playerName.render(10 + text_player.getWidth(), SCREEN_HEIGHT - text_playerName.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				if (jumper.flying || jumper.crash || jumper.landed) text_speed.render(SCREEN_WIDTH - text_speed.getWidth(), arrow.getWidth() + arrow.getHeight() + text_wind.getHeight() + text_speed.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				if (jumper.landed && !jumper.crash) text_distance.render(SCREEN_WIDTH - text_distance.getWidth(), SCREEN_HEIGHT - text_distance.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				caption.render(SCREEN_WIDTH / 2 - caption.getWidth(), SCREEN_HEIGHT - caption.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				//Update screen
				SDL_RenderPresent(gameRenderer);

				//count frame
				framecount++;
				//Graphics rendering delay
				SDL_Delay(50);
			}
			if (tour && endTour)
			{
				//Clear screen
				SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderClear(gameRenderer);
				int max = 0;
				float maxdist = 0;
				for (int i = 0; i < playeramount; i++)
				{
					if (maxdist < distances[i])
					{
						max = i;
						maxdist = distances[i];
					}

				}
				menu_title.loadFromRenderedText("TOURNAMENT RESULTS", color_white, fontSize + 20);
				menu_opt1.loadFromRenderedText("Winner is: " + players[max] + ", who jumped " + trimzero(to_string(distances[max]))+ "m" ,color_white,fontSize+5);


				int optheight = menu_opt1.getHeight();
				int margin = menu_title.getHeight() + fontSize * 2;

				menu_title.render(SCREEN_WIDTH / 2 - menu_title.getWidth() / 2, 20, NULL, 0, NULL, SDL_FLIP_NONE);
				menu_opt1.render(20, margin, NULL, 0, NULL, SDL_FLIP_NONE);
				SDL_RenderPresent(gameRenderer);
				SDL_Delay(3000);
			}
		
}

//manages menu 
void menu(int & choice)
{
	if (!choice)
	{
		SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(gameRenderer);
		int menufontsize = getFontSizeRelativeToResolutionByConstant(30.0);

		menu_title.loadFromRenderedText("SKI JUMP", color_white, menufontsize + 20);
		menu_opt1.loadFromRenderedText("[1] Training", color_white, menufontsize);
		menu_opt2.loadFromRenderedText("[2] Tournament", color_white, menufontsize);
		menu_opt3.loadFromRenderedText("[3] Add player", color_white, menufontsize);
		menu_opt4.loadFromRenderedText("[4] Exit", color_white, menufontsize);

		int optheight = menu_opt1.getHeight();
		int margin = menu_title.getHeight() + menufontsize * 2;

		menu_title.render(SCREEN_WIDTH / 2 - menu_title.getWidth() / 2, 20, NULL, 0, NULL, SDL_FLIP_NONE);
		menu_opt1.render(20, margin, NULL, 0, NULL, SDL_FLIP_NONE);
		menu_opt2.render(20, margin + optheight, NULL, 0, NULL, SDL_FLIP_NONE);
		menu_opt3.render(20, margin + optheight * 2, NULL, 0, NULL, SDL_FLIP_NONE);
		menu_opt4.render(20, margin + optheight * 3, NULL, 0, NULL, SDL_FLIP_NONE);
		caption.render(SCREEN_WIDTH / 2 - caption.getWidth(), SCREEN_HEIGHT - caption.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
		SDL_RenderPresent(gameRenderer);
	}
	else if (choice == 1)
	{
		run();
		SDL_SetRenderDrawColor(gameRenderer, 0x00,0x00, 0x00, 0xFF);
		SDL_RenderClear(gameRenderer);
		choice = 0;
	}
	else if (choice == 2)
	{
		int menufontsize = getFontSizeRelativeToResolutionByConstant(30.0);
		menu_title.loadFromRenderedText("TOURNAMENT - SELECT", color_white, menufontsize + 20);
		int optheight = menu_opt1.getHeight();
		int margin = menu_title.getHeight() + menufontsize * 2;

		fstream playerfile;
		playerfile.open("data/jumpers/jumperlist.txt", ios::out | ios::in);
		playerfile.seekg(0, ios::beg);

		string record;
		const int arr = 30;
		string playerarray[arr];
		int count = 0;
		playerfile.seekg(0, ios::beg);
		while (playerfile >> record)
		{
			if (count < arr) playerarray[count] = record;
			else break;
			count++;
		}
		SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(gameRenderer);
		menu_title.render(SCREEN_WIDTH / 2 - menu_title.getWidth() / 2, 20, NULL, 0, NULL, SDL_FLIP_NONE);
		if (!count)
		{
			menu_opt1.loadFromRenderedText("There are no players to choose.", color_white, menufontsize);
			menu_opt1.render(20, margin, NULL, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderPresent(gameRenderer);
			SDL_Delay(700);
		}
		else {
			int min = 0;
			int max = 0;
			string texts[6];
			for (int i = 0; i < 6; i++)
			{
				texts[i] = "";
			}
			if (count >= 1)
			{
				max = 4;
				if (max > count - 1) max = count - 1;
				texts[0] = parsePlayerArrayToText(playerarray, min, max, arr);
				menu_opt1.loadFromRenderedText(texts[0], color_white, menufontsize);
			}
			if (count >= 6)
			{
				max = 9;
				min = 5;
				if (max > count - 1) max = count - 1;
				texts[1] = parsePlayerArrayToText(playerarray, min, max, arr);
				menu_opt2.loadFromRenderedText(texts[1], color_white, menufontsize);
			}
			if (count >= 11)
			{
				max = 14;
				min = 10;
				if (max > count - 1) max = count - 1;
				texts[2] = parsePlayerArrayToText(playerarray, min, max, arr);
				menu_opt3.loadFromRenderedText(texts[2], color_white, menufontsize);
			}
			
			if (count > 0) menu_opt1.render(20, margin, NULL, 0, NULL, SDL_FLIP_NONE);
			if (count > 5) menu_opt2.render(20, margin + menu_opt1.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
			if (count > 10) menu_opt3.render(20, margin + menu_opt1.getHeight() * 2, NULL, 0, NULL, SDL_FLIP_NONE);
			SDL_RenderPresent(gameRenderer);
			SDL_StartTextInput();
			bool quit = false;
			string nameinput = "Enter numbers separated by comma: ";
			while (!quit)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					//Special key input
					else if (e.type == SDL_KEYDOWN)
					{
						//Handle backspace
						if (e.key.keysym.sym == SDLK_BACKSPACE && nameinput.length() > 0)
						{
							nameinput = " ";
						}
						else if (e.key.keysym.sym == SDLK_RETURN)
						{
							quit = true;
						}
						else if (e.key.keysym.sym == SDLK_ESCAPE)
						{
							quit = true;
							nameinput = " ";
						}
					}
					//Special text input event
					else if (e.type == SDL_TEXTINPUT)
					{
						nameinput += e.text.text;
					}
				}
				SDL_RenderClear(gameRenderer);
				menu_opt4.loadFromRenderedText(nameinput, color_white, menufontsize);
				if (count > 0) menu_opt1.render(20, margin, NULL, 0, NULL, SDL_FLIP_NONE);
				if (count > 5) menu_opt2.render(20, margin + menu_opt1.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				if (count > 10) menu_opt3.render(20, margin + menu_opt1.getHeight() * 2, NULL, 0, NULL, SDL_FLIP_NONE);
				menu_opt4.render(20, margin + menu_opt4.getHeight() * 5, NULL, 0, NULL, SDL_FLIP_NONE);
				caption.render(SCREEN_WIDTH / 2 - caption.getWidth(), SCREEN_HEIGHT - caption.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				SDL_RenderPresent(gameRenderer);
			}
			SDL_Delay(300);
			for (int i = 0; i < 30; i++)
			{
				players[i] = "";
			}
			string temp = "";
			int index = 0;
			nameinput += ',';
			int num = 0;
			for (int i = 0; i < nameinput.size(); i++)
			{
				if (nameinput[i] >= 48 && nameinput[i] <= 57) temp += nameinput[i];
				if (nameinput[i] == ',')
				{
					if (temp != "") num = std::stoi(temp) - 1;
					else num = -1;
					if (num < count && num >= 0)
					{
						players[index] = playerarray[num];
						temp = "";
						index++;
					}
				}
			}
			if (index != 0) run(true);
			SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xFF);
			SDL_RenderClear(gameRenderer);
			choice = 0;
		}

		choice = 0;

	}
	else if (choice == 3)
	{
			int menufontsize = getFontSizeRelativeToResolutionByConstant(30.0);
			menu_title.loadFromRenderedText("PLAYERS", color_white, menufontsize + 20);
			menu_opt1.loadFromRenderedText("Player name: ", color_black, menufontsize);
			int optheight = menu_opt1.getHeight();
			int margin = menu_title.getHeight() + menufontsize * 2;
			bool quit = false;
			char letter = 'a';
			string name = "New player name: ";
			string nameinput = "";
			string lel;
			SDL_SetRenderDrawColor(gameRenderer, 0x00, 0x00, 0x00, 0xFF);
			SDL_RenderClear(gameRenderer);
			SDL_RenderPresent(gameRenderer);
			SDL_StartTextInput();
			while (!quit)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					//Special key input
					else if (e.type == SDL_KEYDOWN)
					{
						//Handle backspace
						if (e.key.keysym.sym == SDLK_BACKSPACE && nameinput.length() > 0)
						{
							nameinput = "";
							SDL_RenderClear(gameRenderer);
						}
						else if (e.key.keysym.sym == SDLK_RETURN)
						{
							quit = true;
						}
						else if (e.key.keysym.sym == SDLK_ESCAPE)
						{
							quit = true;
						}
					}
					//Special text input event
					else if (e.type == SDL_TEXTINPUT)
					{
							nameinput += e.text.text;
					}
				}

				menu_opt1.loadFromRenderedText(name + nameinput, color_white, menufontsize);
				menu_title.render(SCREEN_WIDTH / 2 - menu_title.getWidth() / 2, 20, NULL, 0, NULL, SDL_FLIP_NONE);
				menu_opt1.render(20, margin, NULL, 0, NULL, SDL_FLIP_NONE);
				caption.render(SCREEN_WIDTH / 2 - caption.getWidth(), SCREEN_HEIGHT - caption.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				SDL_RenderPresent(gameRenderer);
			}
			if (checkName(nameinput))
			{
				fstream playerfile;
				playerfile.open("data/jumpers/jumperlist.txt", ios::out | ios::in);
				playerfile.seekg(0, ios::end);
				playerfile << nameinput << endl;
			}
			else
			{
				menu_opt2.loadFromRenderedText("Error. Invalid name.", color_white, menufontsize);
				menu_opt2.render(20, margin + menu_opt1.getHeight(), NULL, 0, NULL, SDL_FLIP_NONE);
				SDL_RenderPresent(gameRenderer);
				SDL_Delay(700);
			}
			
			choice = 0;
			
	}
	else
	{

		choice = 0;
	}

}

//main process of app
int main(int argc, char* args[])
{
	//Load SDL modules
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media (pictures, sound etc)
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
			return 0;
		}
	}
	//Loop flag for handling exiting 
	bool quit = false;
	int choice = 0;
	int insidechoice = 0;

	caption.loadFromRenderedText("tuvrai", color_green, 12);
	
/*	while (!quit)
	{
		switch 
	}
	*/
	//run();
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else
			{
				if (e.type == SDL_KEYDOWN)
				{
					switch (e.key.keysym.sym)
					{
					case SDLK_1:
						choice = 1;
						break;
					case SDLK_2:
						choice = 2;
						break;
					case SDLK_3:
						choice = 3;
						break;
					case SDLK_4:
						quit = true;
					case SDLK_ESCAPE:
						if (choice) choice = 0;
						else quit = true;
						break;
					}
				}
			}

		}
		menu(choice);
		SDL_Delay(50);
	}
	//Destroy objects
	close();
	return 0;
}