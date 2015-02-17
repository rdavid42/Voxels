
#include "Biome.hpp"

Biome::Biome(void) : Octree()
{
	return ;
}

Biome::Biome(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s)
{
	return ;
}

Biome::Biome(Biome const &src) : Octree(src)
{
	return ;
}

Biome::~Biome(void)
{
	return ;
}

Biome
&Biome::operator=(Biome const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Biome const &i)
{
	o	<< "Biome: " << &i;
	return (o);
}
