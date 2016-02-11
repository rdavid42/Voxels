
#include "Constants.hpp"
#include "Octree.hpp"
#include "Block.hpp"
#include "Link.hpp"

Octree::Octree(void)
	: _state(EMPTY), _parent(NULL)
{
	return ;
}

Octree::Octree(Octree const &src)
	: _state(src.getState()), _parent(NULL)
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
		this->_parent = rhs.getParent();
	}
	return (*this);
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

int32_t const &
Octree::getState(void) const
{
	return (this->_state);
}

Octree *
Octree::getParent(void) const
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
	o << "state = " << t.getState();
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
