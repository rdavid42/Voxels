
#include "Block.hpp"
#include "Core.hpp"

Block::Block(void)
{
	_type = AIR;
	return ;
}

Block::~Block(void)
{
	return ;
}

void
Block::setType(uint8_t const &type)
{
	_type = type;
}

uint8_t const &
Block::getType(void) const
{
	return (_type);
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
