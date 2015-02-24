
#include "Block.hpp"

Block::Block(void) : Octree(), c()
{
	return ;
}

Block::Block(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s), c()
{
	return ;
}

Block::Block(Block const &src) : Octree(src), c(src.c)
{
	return ;
}

Block::~Block(void)
{
	return ;
}

void
Block::render(void) const
{
	glColor3f(c.x, c.y, c.z);
	drawCube(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
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
