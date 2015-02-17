
#include "Container.hpp"

Container::Container(void)
{
	int		i;

	i = 0;
	while (i < 20)
	{
		this->stock[i] = NULL;
		i++;
	}
	return ;
}

Container::Container(Container const & src)
{
	*this = src;

	return ;
}

Container::~Container(void)
{
	return ;
}

Container &
Container::operator=(Container const & rhs)
{
	if (this != &rhs)
	{
			//// PUT VALUE HERE
			//// this-> val = rhs.getVal();
	}
	return (*this);
}

Vec3<float>
Container::getFirstBlock(void)
{
	Vec3<float>		ret;

	ret = this->stock[0]->color;
	this->deleteFirst();
	return (ret);
}

void
Container::add(BlockItem const & item)
{
	int			i;
	BlockItem	*toAdd = new BlockItem(item.color);

	i = 0;
	while (i < 20)
	{
		if (this->stock[i] == NULL)
		{
			this->stock[i] = toAdd;
			break ;
		}
		i++;
	}
	return ;
}

void
Container::deleteFirst(void)
{
	int		i;
	int		j;

	i = 0;
	j = 1;
	while (i < 20 && this->stock[j] != NULL && this->stock[i] != NULL)
	{
		this->stock[i] = this->stock[j];
		i++;
		j++;
	}
	this->stock[i] = NULL;
}

std::ostream&
operator<<(std::ostream &o, Container const &i)
{
	o << "Class : Container" << std::endl;
	(void)i;
	return (o);
}
