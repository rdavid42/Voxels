
#include "Constants.hpp"
#include "Octree.hpp"
#include "Block.hpp"
#include "Link.hpp"

Octree::Octree(void)
	: _state(EMPTY), _cube(), _parent(NULL), c(0, 0, 0)
{
	return ;
}

Octree::Octree(Octree const &src)
	: _state(src.getState()), _cube(src.getCube()), _parent(NULL), c(src.c)
{
	return ;
}

Octree::Octree(float const &x, float const &y, float const &z, float const &s)
	: _state(EMPTY), _cube(x, y, z, s), _parent(NULL), c(0, 0, 0)
{
	return ;
}

Octree::Octree(Vec3<float> const &c, float const &x, float const &y, float const &z, float const &s)
	: _state(EMPTY), _cube(x, y, z, s), _parent(NULL), c(c)
{
	return ;
}

Octree::~Octree(void)
{
	return ;
}

Octree &
Octree::operator=(Octree const &rhs)
{
	if (this != &rhs)
	{
		this->_state = rhs.getState();
		this->_cube = rhs.getCube();
	}
	return (*this);
}

// -------------------------------------------------------------------
// Remove current octree from his parent,
// and set it to NULL in the parent.
// You should never call delete on an Octree (ex: delete this->octree;)
// because it doesn't set it to NULL in his parent
// -------------------------------------------------------------------
void
Octree::remove(void)
{
	int			i;
	Octree *	parent = this->_parent;

	if (parent != NULL)
	{
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (parent->getChild(i) == this)
			{
				delete parent->getChild(i);
				parent->setChild(i, NULL);
			}
		}
	}
}
// -------------------------------------------------------------------

Octree *
Octree::backwardSimplification(void)
{
/*	int				i;

	if (_parent && !(_parent->getState() & CHUNK))
	{
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (_parent->getChild(i))
			{
				if (!(_parent->getChild(i)->getState() & GROUND))
					return (this);
			}
			else
				return (this);
		}
		Vec3<float> cl(0, 0, 0);
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (_parent->getChild(i))
			{
				cl.x += _parent->getChild(i)->c.x;
				cl.y += _parent->getChild(i)->c.y;
				cl.z += _parent->getChild(i)->c.z;
				// delete _parent->getChild(i);
				_parent->setChild(i, NULL);
			}
		}
		_parent->setState(_parent->getState() | GROUND);
		_parent->c.x = cl.x / 8.0f;
		_parent->c.y = cl.y / 8.0f;
		_parent->c.z = cl.z / 8.0f;
		return (_parent->backwardSimplification());
	}*/
	return (this);
}

// -------------------------------------------------------------------
// Draw a cube with 12 triangles
// -------------------------------------------------------------------
void
Octree::drawCube(float const &x, float const &y, float const &z, float const &s) const
{
	glBegin(GL_TRIANGLES);
		glVertex3f(x,       y,      z);     // 0
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x,       y,      z + s); // 4
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x,       y,      z);     // 0
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x,       y,      z + s); // 4
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x,       y,      z + s); // 4
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x,       y,      z);     // 0
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x,       y,      z);     // 0
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x,       y,      z + s); // 4
	glEnd();
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Draw the 16 cube edges
// -------------------------------------------------------------------
void
Octree::drawCubeRidges(float const &x, float const &y, float const &z, float const &s) const
{
	glBegin(GL_LINES);
		glVertex3f(x,       y,      z);     // 0
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x,       y,      z);     // 0

		glVertex3f(x,       y,      z);     // 0
		glVertex3f(x,       y,      z + s); // 4
		glVertex3f(x + s,   y,      z);     // 1
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x + s,   y + s,  z);     // 3
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x,       y + s,  z);     // 2
		glVertex3f(x,       y + s,  z + s); // 6

		glVertex3f(x,       y,      z + s); // 4
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x + s,   y,      z + s); // 5
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x + s,   y + s,  z + s); // 7
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x,       y + s,  z + s); // 6
		glVertex3f(x,       y,      z + s); // 4
	glEnd();
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Setters and getters
// -------------------------------------------------------------------
void
Octree::setCube(float const &x, float const &y, float const &z, float const &s)
{
	this->_cube = Cube(x, y, z, s);
}

void
Octree::setState(int32_t const &state)
{
	this->_state = state;
}

void
Octree::setParent(Octree *parent)
{
	this->_parent = parent;
}

Cube const &
Octree::getCube(void) const
{
	return (this->_cube);
}

Cube *
Octree::getCube(void)
{
	return (&this->_cube);
}

int32_t const &
Octree::getState(void) const
{
	return (this->_state);
}

Octree *
Octree::getParent(void)
{
	return (this->_parent);
}

uint32_t
Octree::getDepth(void)
{
	Octree          *tmp = this->_parent;
	uint32_t        depth = 0;

	while (tmp != NULL)
	{
		depth++;
		tmp = tmp->getParent();
	}
	return (depth);
}

std::ostream &
operator<<(std::ostream &o, Octree &t)
{
	uint32_t const      depth = t.getDepth();

	if (t.getParent() == &t)
		o << "Octree: ";
	o   << "state = "
		<< t.getState()
		<< ", "
		<< t.getCube();
	for (uint32_t it = 0; it < CHD_MAX; ++it)
	{
		if (t.getChild(it) != NULL)
		{
			o << std::endl;
			for (uint32_t i = 0; i < depth; i++)
				o << "-";
			o   << "- child "
				<< it
				<< ": "
				<< *(t.getChild(it));
		}
	}
	return (o);
}

/*

SUBDIVISION PROCESS

i = 0;
000 -> 0
nx = x + 0 * s;
ny = y + 0 * s;
nz = z + 0 * s;

i = 1;
001 -> 1
nx = x + 1 * s;
ny = y + 0 * s;
nz = z + 0 * s;

i = 2;
010 -> 2
nx = x + 0 * s;
ny = y + 1 * s;
nz = z + 0 * s;

i = 3;
011 -> 3
nx = x + 1 * s;
ny = y + 1 * s;
nz = z + 0 * s;

i = 4;
100 -> 4
nx = x + 0 * s;
ny = y + 0 * s;
nz = z + 1 * s;

i = 5;
101 -> 5
nx = x + 1 * s;
ny = y + 0 * s;
nz = z + 1 * s;

i = 6;
110 -> 6
nx = x + 0 * s;
ny = y + 1 * s;
nz = z + 1 * s;

i = 7;
111 -> 7
nx = x + 1 * s;
ny = y + 1 * s;
nz = z + 1 * s;

get bit:
z -> i & 1
y -> (i >> 1) & 1
x -> (i >> 2) & 1

*/

