
#include "Constants.hpp"
#include "Octree.hpp"
#include "Chunk.hpp"
#include "Block.hpp"

uint32_t
Octree::block_depth = BLOCK_DEPTH;

uint32_t
Octree::chunk_depth = CHUNK_DEPTH;

Octree::Octree(void)
	: _state(0), _cube(), _parent(NULL), c(), generated(false), iterated(false)
{
	for (uint32_t i = 0; i < CHD_MAX; ++i)
		this->_children[i] = NULL;
}

Octree::Octree(Octree const &src)
	: _state(src.getState()), _cube(src.getCube()), _parent(NULL), c(), generated(false), iterated(false)
{
	for (uint32_t i = 0; i < CHD_MAX; ++i)
		this->_children[i] = NULL;
}

Octree::Octree(float const &x, float const &y, float const &z, float const &s)
	: _state(0), _cube(x, y, z, s), _parent(NULL), c(), generated(false), iterated(false)
{
	for (uint32_t i = 0; i < CHD_MAX; ++i)
		this->_children[i] = NULL;
}

Octree::~Octree(void)
{
	for (uint32_t i = 0; i < CHD_MAX; ++i)
	{
		if (this->_children[i] != NULL)
			delete this->_children[i];
		this->_children[i] = NULL;
	}
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
// Divides the current octree into four children
// It's better practice to create only useful children
// -------------------------------------------------------------------
int
Octree::subdivide(void)
{
	for (uint32_t i = 0; i < CHD_MAX; ++i)
		this->createChild(i);
	return (0);
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// NULL check and no given coordinates
// -------------------------------------------------------------------
int
Octree::createChild(uint32_t const &i)
{
	if (this->_children[i] == NULL)
	{
		float const     s = this->_cube.getS() / 2.0f;

		this->_children[i] = new Octree(this->_cube.getX() + ((i >> 0) & MASK_1) * s,
										this->_cube.getY() + ((i >> 1) & MASK_1) * s,
										this->_cube.getZ() + ((i >> 2) & MASK_1) * s,
										s);
		this->_children[i]->setParent(this);
		return (1);
	}
	return (0);
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Should be useless for this project
// since world octree's size is huge
// -------------------------------------------------------------------
void
Octree::grow(uint32_t const &gd) // gd : grow direction [0, 1, 2, 3, 4, 5, 6, 7]
{
	if (this->_parent != NULL)
		return ;
	this->block_depth++;
	this->chunk_depth++;
	this->_parent = new Octree( this->_cube.getX() - (~(gd >> 0) & MASK_1)	* this->_cube.getS(),
								this->_cube.getY() - (~(gd >> 1) & MASK_1)	* this->_cube.getS(),
								this->_cube.getZ() - (~(gd >> 2) & MASK_1)	* this->_cube.getS(),
								this->_cube.getS()							* 2);
	this->_parent->setChild(~gd & 3, this);
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Creates a child with no NULL check and coordinates given
// -------------------------------------------------------------------
void
Octree::createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s)
{
	this->_children[i] = new Octree(x, y, z, s);
	this->_children[i]->setParent(this);
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Creates a child with octree state specified
// -------------------------------------------------------------------
void
Octree::createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s, uint32_t const &state)
{
	switch (state)
	{
		case EMPTY:
			this->_children[i] = new Octree(x, y, z, s);
			break;
		case CHUNK:
			this->_children[i] = new Chunk(x, y, z, s);
			break;
		case BLOCK:
			this->_children[i] = new Block(x, y, z, s);
			break;
		default:
			this->_children[i] = new Octree(x, y, z, s);
	}
	this->_children[i]->setParent(this);
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Search the octree with a point and returns a pointer
// on the deepest child found.
// -------------------------------------------------------------------
Octree *
Octree::search(float const &x, float const &y, float const &z)
{
	Octree *		child;
	int				i;

	if (this->_cube.vertexInside(x, y, z))
	{
		child = NULL;
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (this->_children[i] != NULL)
			{
				if ((child = this->_children[i]->search(x, y, z)) != NULL)
					break;
			}
		}
		return (child == NULL ? this : child);
	}
	else
		return (NULL);
}
// -------------------------------------------------------------------

Octree *
Octree::search(float const &x, float const &y, float const &z, int const &state)
{
	Octree *		child;
	int				i;

	if (this->_cube.vertexInside(x, y, z))
	{
		if (this->_state == state)
			return (this);
		child = NULL;
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (this->_children[i] != NULL)
			{
				if ((child = this->_children[i]->search(x, y, z)) != NULL)
					break;
			}
		}
		return (child == NULL ? this : child);
	}
	else
		return (NULL);
}

// -------------------------------------------------------------------
// Remove current octree from his parent,
// and set it to NULL in the parent.
// You should never call delete on an octree (ex: delete this->octree;)
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

// -------------------------------------------------------------------
// Insert an octree in an another
// -------------------------------------------------------------------
Octree *
Octree::insert(float const &x, float const &y, float const &z, uint32_t const &depth, uint32_t const &state, Vec3<float> const &c)
{
	// size never changes for children.
	float const     s = this->_cube.getS() / 2.0f;
	float           nx;
	float           ny;
	float           nz;

	if (depth == 0)
	{
		// max depth reached, put values here
		this->setState(state);
		this->c = c;
		// std::cerr << "insert deepest: " << this << std::endl;
		return (this);
	}
	else if (depth > 0)
	{
		int				i;

		for (i = 0; i < CHD_MAX; ++i)
		{
			if (this->_children[i] == NULL)
			{
				/*
				** precomputing child coordinates in order to check if the vertex is inside.
				** new_dim = dim + coefficient * size;
				*/
				nx = this->_cube.getX() + ((i >> 0) & MASK_1) * s;
				ny = this->_cube.getY() + ((i >> 1) & MASK_1) * s;
				nz = this->_cube.getZ() + ((i >> 2) & MASK_1) * s;
				if (x >= nx && x < nx + s && y >= ny && y < ny + s && z >= nz && z < nz + s)
				{
					if ((depth - 1) == 0)
						this->createChild(i, nx, ny, nz, s, state);
					else
						this->createChild(i, nx, ny, nz, s);
					return (this->_children[i]->insert(x, y, z, depth - 1, state, c));
				}
			}
			else if (this->_children[i]->getCube()->vertexInside(x, y, z))
				return (this->_children[i]->insert(x, y, z, depth - 1, state, c));
		}
	}
	// std::cerr << "not created, depth: " << depth << std::endl;
	return (NULL);
}
// -------------------------------------------------------------------

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
// Draw the 16 ridges of a cube, usually for debug purposes
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
// no comment yet
// -------------------------------------------------------------------
void
Octree::renderGround(void) const
{
	// float			tmp;

	if (c.x != 0.0f || c.y != 0.0f || c.z != 0.0f)
	{
		glColor3f(c.x, c.y, c.z);
		drawCubeRidges(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
	}
	if (this->_state == BLOCK)
	{
		glColor3f(c.x, c.y, c.z);
		drawCube(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
	}
#ifdef DEBUG
	else if (this->_state == CHUNK)
	{
		glColor3f(c.x, c.y, c.z);		
		drawCubeRidges(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
	}
#endif
	else if (this->_parent == NULL)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		drawCubeRidges(this->_cube.getX(), this->_cube.getY(), this->_cube.getZ(), this->_cube.getS());
	}
	for (int i = 0; i < CHD_MAX; ++i)
	{
		if (this->_children[i] != NULL)
			this->_children[i]->renderGround();
	}
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

void
Octree::setChild(uint32_t const &i, Octree *const child)
{
	this->_children[i] = child;
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
Octree::getChild(uint32_t const &i) const
{
	return (this->_children[i]);
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

