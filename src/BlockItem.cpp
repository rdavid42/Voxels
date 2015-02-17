
#include "BlockItem.hpp"
#include "Vec3.hpp"

BlockItem::BlockItem(void) : Item()
{
	return ;
}


BlockItem::BlockItem(Vec3<float> color) : Item(), color(color)
{
	return ;
}

BlockItem::BlockItem(BlockItem const & src) : Item()
{
	*this = src;

	return ;
}

BlockItem::~BlockItem(void)
{
	return ;
}

BlockItem &
BlockItem::operator=(BlockItem const & rhs)
{
	if (this != &rhs)
	{
		this->color = rhs.color;
			//// PUT VALUE HERE
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

std::ostream&
operator<<(std::ostream &o, BlockItem const &i)
{
	o << "Class : BlockItem" << std::endl;
	(void)i;
	return (o);
}
