#include "Vec2D.h"

//template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>>
Vec2D::Vec2D(double x, double y) :x{x}, y{y }
{
	;
}


Vec2D::~Vec2D()
{
}

Vec2D Vec2D::operator+(Vec2D& other)
{
	return{ other.x + x, other.y + y };
}
Vec2D& Vec2D::operator+=(Vec2D& other)
{
	x += other.x, y += other.y;
	return *this;
}
Vec2D Vec2D::operator-(Vec2D& other)
{
	return{ x - other.x, y - other.y };
}
Vec2D& Vec2D::operator-=(Vec2D& other)
{
	x -= other.x, y -= other.y;
	return *this;
}
double Vec2D::operator*(Vec2D& other) //dot product
{
	return x * other.x + y * other.y;
}

Vec2D Vec2D::operator*(double val)
{
	return{ x * val, y * val };
}
Vec2D& Vec2D::operator*=(double val)
{
	x *= val, y *= val;
	return *this;
}
Vec2D Vec2D::operator/(double val)
{
	if (val == 0) throw "Cannot Divide By Zero";
	return{ x / val, y / val };
}
Vec2D& Vec2D::operator/=(double val)
{
	if (val == 0) throw "Cannot Divide By Zero";
	x /= val, y /= val;
	return *this;
}

Vec2D Vec2D::operator-()
{
	return{ -x, -y };
}
double Vec2D::Magnitude()
{
	return sqrt(Vec2D::MagnitudeSQ());
}
double Vec2D::MagnitudeSQ()
{
	return x*x + y*y;
}
double Vec2D::Angle()
{
	return atan2(y, x);
}
Vec2D Vec2D::Unit()
{
	return{ x / Magnitude(), y / Magnitude() };
}
Vec2D Vec2D::Rotate(double theta)
{
	return{ x*cos(theta) - y*sin(theta), x*sin(theta) + y*cos(theta) };
}

Vec2D Vec2D::Truncate(double length)
{
	return Unit()*length;
}