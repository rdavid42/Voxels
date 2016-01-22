
#include "Core.hpp"
#include "Chunk.hpp"

Chunk::Chunk(void) : Link(), generated(false)
{
	return ;
}

Chunk::Chunk(float const &x, float const &y, float const &z, float const &s) : Link(x, y, z, s), generated(false)
{
	return ;
}

Chunk::~Chunk(void)
{
	return ;
}

void
Chunk::render(Core &core) const
{
	int			i;

	// std::cerr << _cube.getS() << std::endl;
	// core.ms.translate(_cube.getX(), _cube.getY(), _cube.getZ());
	// core.ms.push();
	// if (generated)
	// {
		for (i = 0; i < CHD_MAX; ++i)
		{
			if (this->_children[i] != NULL)
				this->_children[i]->render(core);
		}
	// }
	// core.ms.pop();
}

Chunk
&Chunk::operator=(Chunk const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Chunk const &i)
{
	o	<< "Chunk: " << &i;
	return (o);
}
