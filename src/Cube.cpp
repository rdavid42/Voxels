
#include "Cube.hpp"

Cube::Cube(void)
	: _x(0), _y(0), _z(0), _s(0)
{
	return ;
}

Cube::Cube(float const &x, float const &y, float const &z, float const &s)
	: _x(x), _y(y), _z(z), _s(s)
{
	return ;
}

Cube::Cube(Cube const &src)
	: _x(src.getX()), _y(src.getY()), _z(src.getZ()), _s(src.getS())
{
	return ;
}

Cube::~Cube(void)
{
	return ;
}

bool
Cube::vertexInside(float const &x, float const &y, float const &z)
{
	if (x >= _x && x < _x + _s
		&& y >= _y && y < _y + _s
		&& z >= _z && z < _z + _s)
		return (true);
	return (false);
}

float const
&Cube::getX(void) const
{
	return (this->_x);
}

float const
&Cube::getY(void) const
{
	return (this->_y);
}

float const
&Cube::getZ(void) const
{
	return (this->_z);
}

float const
&Cube::getS(void) const
{
	return (this->_s);
}

void
Cube::setX(float const &x)
{
	this->_x = x;
}

void
Cube::setY(float const &y)
{
	this->_y = y;
}

void
Cube::setZ(float const &z)
{
	this->_z = z;
}

void
Cube::setS(float const &s)
{
	this->_s = s;
}

Cube
&Cube::operator=(Cube const &rhs)
{
	if (this != &rhs)
	{
		this->_x = rhs.getX();
		this->_y = rhs.getY();
		this->_z = rhs.getZ();
		this->_s = rhs.getS();
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Cube const &i)
{
	o	// << &i
		<< "x:"
		<< i.getX()
		<< ", y:"
		<< i.getY()
		<< ", z:"
		<< i.getZ()
		<< ", s:"
		<< i.getS();
	return (o);
}
