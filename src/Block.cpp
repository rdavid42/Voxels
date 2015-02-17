
#include "Block.hpp"

Block::Block(void) : Octree()
{
	return ;
}

Block::Block(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s)
{
	return ;
}

Block::Block(Block const &src) : Octree(src)
{
	return ;
}

Block::~Block(void)
{
	return ;
}

Block
&Block::operator=(Block const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Block const &i)
{
	o	<< "Block: " << &i;
	return (o);
}
