
#include "Constants.hpp"
#include "Octree.hpp"
#include "Block.hpp"
#include "Link.hpp"

Octree::Octree(void)
	: _state(EMPTY), _cube(), _parent(NULL)
{
	return ;
}

Octree::Octree(Octree const &src)
	: _state(src.getState()), _cube(src.getCube()), _parent(NULL)
{
	return ;
}

Octree::Octree(float const &x, float const &y, float const &z, float const &s)
	: _state(EMPTY), _cube(x, y, z, s), _parent(NULL)
{
	return ;
}

Octree::~Octree(void)
{
	_state = ERROR;
	_parent = 0;
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
// Remove current octree from his parent, and set it to NULL
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
/*
Cube *
Octree::getCube(void)
{
	return (&this->_cube);
}
*/
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
	Octree			*tmp = this->_parent;
	uint32_t		depth = 0;

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
