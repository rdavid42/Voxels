
#include "Item.hpp"

Item::Item(void)
{
	return ;
}

Item::Item(Item const & src)
{
	*this = src;

	return ;
}

Item::~Item(void)
{
	return ;
}

Item &
Item::operator=(Item const & rhs)
{
	if (this != &rhs)
	{
			//// PUT VALUE HERE
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

std::ostream&
operator<<(std::ostream &o, Item const &i)
{
	o << "Class : Item" << std::endl;
	(void)i;
	return (o);
}
