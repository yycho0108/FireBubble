#pragma once
#include <cmath>

class Vec2D
{
public:
	double x, y;
	Vec2D(double x=0, double y=0);
	~Vec2D();
	Vec2D operator+(Vec2D& other);
	Vec2D& operator+=(Vec2D& other);
	Vec2D operator-(Vec2D& other);
	Vec2D& operator-=(Vec2D& other);
	double operator*(Vec2D& other);
	//Vec2D operator/(Vec2D& other);
	//Vec2D& operator/=(Vec2D& other); NO VECTOR DIVISION

	Vec2D operator*(double val);
	Vec2D& operator*=(double val);
	Vec2D operator/(double val);
	Vec2D& operator/=(double val);

	Vec2D operator-(); //flip sign

	double Magnitude();
	double MagnitudeSQ();
	double Angle();
	Vec2D Unit();
	Vec2D Rotate(double theta);
	Vec2D Truncate(double length);
};

