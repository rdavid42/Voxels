
#include "Container.hpp"
#include "Camera.hpp"

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
Container::drawInventory(void)
{
	int		i;
	int		j;
	int		tab;
	int		size;

	size = 35;
	tab = 10;
	i = 0;
	j = 1;
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	glVertex2i(10, 120);
	glVertex2i(10, 1030);
	glVertex2i(140, 120);
	glVertex2i(140, 120);
	glVertex2i(140, 1030);
	glVertex2i(10, 1030);

	while (i < 20 && this->stock[i] != NULL)
	{
		glColor3f(this->stock[i]->color.x, this->stock[i]->color.y, this->stock[i]->color.z);
		glVertex2i(20, 120 + (i * (size + tab) + tab));
		glVertex2i(20, 120 + (j * (size + tab)));
		glVertex2i(130, 120 + (i * (size + tab) + tab));
		glVertex2i(130, 120 + (i * (size + tab) + tab));
		glVertex2i(130, 120 + (j * (size + tab)));
		glVertex2i(20, 120 + (j * (size + tab)));
		i++;
		j++;
	}
	glEnd();
}

void
Container::deleteFirst(void)
{
	int		i;
	int		j;

	i = 0;
	j = 1;
	while (j < 20 && this->stock[j] && this->stock[i])
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
