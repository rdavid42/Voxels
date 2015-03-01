
#include "Block.hpp"

Block::Block(void) : Octree(), c()
{
	this->_state = GROUND;
	return ;
}

Block::Block(Vec3<float> const &c, float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s), c(c)
{
	this->_state = GROUND;
	return ;
}

Block::Block(Block const &src) : Octree(src), c(src.c)
{
	this->_state = GROUND;
	return ;
}

Block::~Block(void)
{
	return ;
}

Octree *
Block::search(float const &x, float const &y, float const &z)
{
	if (this->_cube.vertexInside(x, y, z))
		return (this);
	return (NULL);
}
// -------------------------------------------------------------------

Octree *
Block::search(float const &x, float const &y, float const &z, int const &state)
{
	if (this->_cube.vertexInside(x, y, z) && this->_state == state)
		return (this);
	return (NULL);
}

Octree *
Block::insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, Vec3<float> const &c)
{
	(void)x;
	(void)y;
	(void)z;
	(void)depth;
	(void)state;
	(void)c;
	return (this);
}

void
Block::render(void) const
{
#ifdef MARCHING_CUBES
	int			i, j;

	glColor3f(c.x, c.y, c.z);
	glBegin(GL_TRIANGLES);
	for (i = 0; i < this->n; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			glVertex3f(t[i].p[j].x, t[i].p[j].y, t[i].p[j].z);
		}
	}
	glEnd();
#else
	glColor3f(c.x, c.y, c.z);
	drawCube(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
#endif
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
