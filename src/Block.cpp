
#include "Block.hpp"
#include "Core.hpp"

Block::Block(void) : Octree()
{
	type = NONE;
	return ;
}

Block::Block(int const &t) : Octree(), type(t)
{
	return ;
}

Block::~Block(void)
{
	return ;
}

Chunk *
Block::getChunk(void)
{
	if (_parent != NULL)
		return (_parent->getChunk());
	return (NULL);
}

void
Block::remove(void)
{
	if (_parent != NULL)
		_parent->deleteChild(this);
	return ;
}

Octree *
Block::search(float const &x, float const &y, float const &z)
{
	if (getCube().vertexInside(x, y, z))
		return (this);
	return (NULL);
}
// -------------------------------------------------------------------

Octree *
Block::search(float const &x, float const &y, float const &z, int const &state, bool const &)
{
	if (getCube().vertexInside(x, y, z) && _state == state)
		return (this);
	return (NULL);
}

Octree *
Block::insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, int const &type)
{
	if (getCube().vertexInside(x, y, z))
	{
		this->type = type;
		setState(state);
	}
	else
		std::cerr <<  depth << std::endl;
	return (this);
}

Cube
Block::getCube(void) const
{
	int				i;
	Cube			pcube = _parent->getCube();
	Cube			cube;

	cube.setS(pcube.getS() / 2.0f);
	for (i = 0; i < CHD_MAX; ++i)
	{
		if (_parent->getChild(i) == this)
		{
			cube.setX(pcube.getX() + ((i >> 0) & MASK_1) * cube.getS());
			cube.setY(pcube.getY() + ((i >> 1) & MASK_1) * cube.getS());
			cube.setZ(pcube.getZ() + ((i >> 2) & MASK_1) * cube.getS());
		}
	}
	return (cube);
}

void
Block::render(Core &core) const
{
	(void)core;
}

void
Block::renderRidges(Core &core) const
{
	Cube		cube;

	cube = getCube();
	(void)core;
	glBindVertexArray(core.selectionVao);
	core.ms.push();
	core.ms.translate(cube.getX(), cube.getY(), cube.getZ());
	core.ms.scale(cube.getS(), cube.getS(), cube.getS());
		glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
		glDrawElements(GL_LINES, core.selectionIndicesSize, GL_UNSIGNED_SHORT, (void *)0);
	core.ms.pop();
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
