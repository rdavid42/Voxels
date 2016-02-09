
#include "Link.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "Core.hpp"

Link::Link(void) : Octree()
{
	for (int i = 0; i < CHD_MAX; ++i)
		_children[i] = NULL;
}

Link::Link(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s)
{
	for (int i = 0; i < CHD_MAX; ++i)
		_children[i] = NULL;
}

Link::~Link(void)
{
	for (int i = 0; i < CHD_MAX; ++i)
	{
		if (_children[i] != NULL)
			delete _children[i];
		_children[i] = NULL;
	}
}

Chunk *
Link::getChunk(void)
{
	if (_parent != NULL)
		return (_parent->getChunk());
	return (NULL);
}

void
Link::deleteChild(Octree *child)
{
	// int			childCount;

	// childCount = 0;
	if (child != NULL)
	{
		for (int i = 0; i < CHD_MAX; ++i)
		{
			if (_children[i] != NULL && _children[i] == child)
			{
				delete _children[i];
				_children[i] = NULL;
			}
	/*		if (_children[i] != NULL)
				childCount++;*/
		}
	}
/*	if (childCount == 0)
	{
		if (_parent != NULL)
			_parent->deleteChild(this);
	}*/
}

void
Link::remove(void)
{
	if (_parent != NULL)
	{
		_parent->deleteChild(this);
	}
}

// -------------------------------------------------------------------
// Search the octree with a point and returns a pointer
// on the deepest child found.
// -------------------------------------------------------------------
Octree *
Link::search(float const &x, float const &y, float const &z)
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
Link::search(float const &x, float const &y, float const &z, int const &state, bool const &allowOutside)
{
	Octree *		child;
	int				i;

	if (_cube.vertexInside(x, y, z))
	{
		if (_state == state)
			return (this);
		child = NULL;
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (_children[i] != NULL)
			{
				if ((child = _children[i]->search(x, y, z, state, allowOutside)) != NULL)
					break;
			}
		}
		return (child == NULL ? NULL : child);
	}
	else
	{
		if (allowOutside)
		{
			if (_parent != NULL)
			{
				// _parent->search(x, y, z, state, allowOutside);
				return (NULL);
			}
		}
	}
	return (NULL);
}

// -------------------------------------------------------------------
// Creates a child with octree state specified
// -------------------------------------------------------------------
void
Link::createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s, int32_t const &state)
{
	if (state & CHUNK)
		_children[i] = new Chunk(x, y, z, s);
	else if (state & BLOCK)
		_children[i] = new Block(x, y, z, s);
	else
		_children[i] = new Link(x, y, z, s);
	_children[i]->setParent(this);
}
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Insert an Octree in another
// -------------------------------------------------------------------
Octree *
Link::insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, int const &type)
{
	// size never changes for children.
	float			nx;
	float			ny;
	float			nz;

	if (depth == 0)
	{
		setState(state);
		return (this);
	}
	else if (depth > 0)
	{
		int				i;
		float const		s = _cube.getS() / 2.0f;

		for (i = 0; i < CHD_MAX; ++i)
		{
			if (_children[i] == NULL)
			{
				/*
				** precomputing child coordinates in order to check if the vertex is inside.
				** new_dim = dim + coefficient * size;
				*/
				nx = _cube.getX() + ((i >> 0) & MASK_1) * s;
				ny = _cube.getY() + ((i >> 1) & MASK_1) * s;
				nz = _cube.getZ() + ((i >> 2) & MASK_1) * s;
				if (x >= nx && x < nx + s && y >= ny && y < ny + s && z >= nz && z < nz + s)
				{
					if ((depth - 1) == 0)
						createChild(i, nx, ny, nz, s, state);
					else
						createChild(i, nx, ny, nz, s, LINK);
					return (_children[i]->insert(x, y, z, depth - 1, state, type));
				}
			}
			else if (_children[i]->getCube()->vertexInside(x, y, z))
			{
				return (_children[i]->insert(x, y, z, depth - 1, state, type));
			}
		}
	}
	return (NULL);
}
// -------------------------------------------------------------------

void
Link::render(Core &core) const
{
	int				i;

	(void)core;
	for (i = 0; i < CHD_MAX; ++i)
	{
		if (_children[i] != 0)
			_children[i]->render(core);
	}
}

void
Link::renderLines(Core &core) const
{
	int				i;

	(void)core;
	for (i = 0; i < CHD_MAX; ++i)
	{
		if (_children[i] != 0)
			_children[i]->renderLines(core);
	}
}

void
Link::renderRidges(Core &core) const
{
	int				i;

	(void)core;
	for (i = 0; i < CHD_MAX; ++i)
	{
		if (_children[i] != 0)
			_children[i]->renderRidges(core);
	}
}

Octree *
Link::getChild(uint32_t const &i)
{
	return (_children[i]);
}

void
Link::setChild(uint32_t const &i, Octree *const child)
{
	_children[i] = child;
}

Link &
Link::operator=(Link const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream &
operator<<(std::ostream &o, Link const &i)
{
	o	<< "Link: " << &i;
	return (o);
}
