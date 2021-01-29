#include "Jumper.h"
#include <cmath>

Jumper::Jumper()
{
	x = 0.0;
	y = 0.0;
	vx = 0.0;
	vy = 0.0;
	tx = 0;
	ty = 0;
	angle = 0;
	hillangle = 0;
	len = 0;
}

int Jumper::getX()
{
	return x;
}

void Jumper::setX(int val)
{
	x = val;
}

void Jumper::moveX(int val)
{
	x += val;
}

int Jumper::getY()
{
	return y;
}

void Jumper::setY(int val)
{
	y = val;
}

void Jumper::moveY(int val)
{
	y += val;
}

void Jumper::changeAngle(int val)
{
	angle += val;
	if (angle > 90) angle = 90;
	else if (angle < -90) angle = -90;
}

void Jumper::setAngle(int val)
{
	if (val <90 && val > -90) angle = val;
}

int Jumper::getAngle()
{
	return angle;
}