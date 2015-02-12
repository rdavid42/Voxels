
#include "Vec3f.hpp"

Vec3f::Vec3f(void)
	: x(0.0f), y(0.0f), z(0.0f)
{
	return ;
}

Vec3f::Vec3f(float const &x, float const &y, float const &z)
	: x(x), y(y), z(z)
{
	return ;
}

Vec3f::~Vec3f(void)
{
	return ;
}

Vec3f
Vec3f::crossProduct(Vec3f const &v) const
{
	Vec3f	r;

	r.x = this->y * v.z - this->z * v.y;
	r.y = this->z * v.x - this->x * v.z;
	r.z = this->x * v.y - this->y * v.x;
	return (r);
}

void
Vec3f::normalize(void)
{
	float const	h = sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
	this->x = this->x / h;
	this->y = this->y / h;
	this->z = this->z / h;
}

Vec3f
&Vec3f::operator=(Vec3f const &rhs)
{
	if (this != &rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		this->z = rhs.z;
	}
	return (*this);
}

Vec3f
Vec3f::operator+(Vec3f const &rhs)
{
	return (Vec3f(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z));
}

Vec3f
Vec3f::operator*(Vec3f const &rhs)
{
	return (Vec3f(this->x * rhs.x, this->y * rhs.y, this->z * rhs.z));
}

Vec3f
Vec3f::operator*(float const &rhs)
{
	return (Vec3f(this->x * rhs, this->y * rhs, this->z * rhs));
}

Vec3f
&Vec3f::operator+=(Vec3f const &rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	return (*this);
}

Vec3f
&Vec3f::operator-=(Vec3f const &rhs)
{
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Vec3f const &i)
{
	o	<< "Vec3f - " << &i << " - x: " << i.x << ", y: " << i.y << ", z: " << i.z;
	return (o);
}
