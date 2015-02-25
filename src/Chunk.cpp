
#include "Chunk.hpp"

Chunk::Chunk(void) : Link(), generated(false)
{
	this->_state = CHUNK;
	return ;
}

Chunk::Chunk(float const &x, float const &y, float const &z, float const &s) : Link(x, y, z, s), generated(false)
{
	this->_state = CHUNK;
	return ;
}

Chunk::Chunk(Chunk const &src) : Link(src), generated(false)
{
	this->_state = CHUNK;
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
#ifdef DEBUG
	if (this->generated)
		glColor3f(0.7f, 0.5f, 0.0f);
	else
		glColor3f(1.0f, 0.0f, 0.0f);
	drawCubeRidges(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
#endif
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
