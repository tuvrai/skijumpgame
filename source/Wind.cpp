#include "Wind.h"
#include <cmath>

Wind::Wind(float xx, float yy)
{
	//maximum and minimum calibration
	if (xx > 2) xx = 2;
	else if (xx < -2) xx = -2;
	else if (xx == 0) xx += 0.001;
	if (yy > 2) yy = 2;
	else if (yy < -2) yy = -2;
	else if (yy == 0) yy += 0.001;
	this->x = xx;
	this->y = yy;
	double natan = 0;
	int modif = 0;
	if (xx > 0 && y > 0)
	{
		modif = 0;
		natan = atan(y / x);
	}
	if (xx < 0 && yy>0)
	{
		modif = 90;
		natan = atan(x / y);
	}
	else if (xx < 0 && yy < 0)
	{
		modif = 180;
		natan = atan(y / x);
	}
	else if (xx > 0 && yy < 0)
	{
		modif = 270;
		natan = atan(x / y);
	}
	else if (xx == 0 && yy == 0)
	{
		modif = 0;
		natan = 0;
	}
	natan = abs(natan);
	natan *= 180;
	natan /= 3.14159265358979326846;
	this->angle =-static_cast<int>(modif + natan);

	force = sqrt(xx * xx + yy * yy);
}

int Wind::getAngle()
{
	return angle;
}

float Wind::getX()
{
	return x;
}

float Wind::getY()
{
	return y;
}
float Wind::getForce()
{
	return force;
}