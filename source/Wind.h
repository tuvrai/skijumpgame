#pragma once
class Wind
{
private:
	//wind in x-direction
	float x;
	//wind in y-direction
	float y;
	//angle of net wind force (clockwise direction)
	int angle;
	//net wind force
	float force;
public:
	//wind constructor
	Wind(float xx, float yy);
	//returns wind x-speed
	float getX();
	//returns wind y-speed
	float getY();
	//returns net wind force angle (clockwise direction)
	int getAngle();
	//returns wind force
	float getForce();
};

