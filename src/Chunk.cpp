
#include "Chunk.hpp"

Chunk::Chunk(void) : Octree()
{
	return ;
}

Chunk::Chunk(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s)
{
	return ;
}

Chunk::Chunk(Chunk const &src) : Octree(src)
{
	return ;
}

Chunk::~Chunk(void)
{
	return ;
}

void
Chunk::render(void) const
{
	int			i;

	glColor3f(c.x, c.y, c.z);
	drawCubeRidges(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());

	for (i = 0; i < CHD_MAX; ++i)
	{
		if (this->_children[i] != NULL)
			this->_children[i]->render();
	}
}

Chunk
&Chunk::operator=(Chunk const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Chunk const &i)
{
	o	<< "Chunk: " << &i;
	return (o);
}
