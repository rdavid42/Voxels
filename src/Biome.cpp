
#include "Biome.hpp"

Biome::Biome(void) : Link()
{
	this->_state = BIOME;
	return ;
}

Biome::Biome(float const &x, float const &y, float const &z, float const &s) : Link(x, y, z, s)
{
	this->_state = BIOME;
	return ;
}

Biome::Biome(Biome const &src) : Link(src)
{
	this->_state = BIOME;
	return ;
}

Biome::~Biome(void)
{
	return ;
}

Biome &
Biome::operator=(Biome const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream &
operator<<(std::ostream &o, Biome const &i)
{
	o	<< "Biome: " << &i;
	return (o);
}
