#pragma once

//structure managing x,y position or point
struct pos
{
	int x;
	int y;
};

class Jumper
{
private:
	//jumpers x position
	float x;
	//jumpers y position
	float y;
	//jumper's angle of rotation in degrees
	int angle;
public:
	//default constructor
	Jumper();

	//jumpers texture x-width
	int tx;
	//jumper's texture y-width
	int ty;
	//jumper's texture corner-centre radius
	float len;
	//speed modifier
	double vconst;
	//speed in x direction
	float vx;
	//speed in y direction
	float vy;
	//returns jumper x position
	int getX();
	//sets x to given value
	void setX(int val);
	//changes x value
	void moveX(int val);
	//returns jumper y position
	int getY();
	//sets y to given value
	void setY(int val);
	//changes y value
	void moveY(int val);
	//flag indicating whether the process of jump is ongoing
	bool started = false;
	//flag indicating whether the jumper has landed on the hill
	bool landed = false;
	//flag indicating whether the jumper is in air
	bool flying = false;
	//flag indicating whether the jumper had crashed during take-off on ramp
	bool crash = false;
	//changes angle of jumper by given value
	void changeAngle(int val);
	//sets rotation angle to given value
	void setAngle(int val);
	//returns jumper's angle
	int getAngle();
	//coordinates jumper's angle while sliding on ramp
	void checkSlide();
	//coordinates jumper's angle while sliding on hill
	void checkHill();
	//checks whether the jumper had landed on the hill and reacts to it
	void checkLand(int x, int y);
	//gets coordinates of part of ski, which is closest to the ground
	pos getSkiEdge();
	//angle of hill straight below the jumper
	int hillangle = 0;
	//distance jumped
	float distance = 0.0;
};

