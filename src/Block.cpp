
#include "Block.hpp"

Block::Block(void) : Octree(), c()
{
	this->_state = BLOCK;
	return ;
}

Block::Block(Vec3<float> const &color) : Octree(), c(color)
{
	this->_state = BLOCK;
	return ;
}

Block::Block(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s), c()
{
	this->_state = BLOCK;
	return ;
}

Block::Block(Block const &src) : Octree(src), c(src.c)
{
	this->_state = BLOCK;
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
