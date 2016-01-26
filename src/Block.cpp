
#include "Block.hpp"
#include "Core.hpp"

Block::Block(void) : Octree()
{
	return ;
}

Block::Block(float const &x, float const &y, float const &z, float const &s) : Octree(x, y, z, s)
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
Block::destroy(void)
{
	if (_parent != NULL)
		_parent->deleteChild(this);
	return ;
}

Octree *
Block::search(float const &x, float const &y, float const &z)
{
	if (this->_cube.vertexInside(x, y, z))
		return (this);
	return (NULL);
}
// -------------------------------------------------------------------

Octree *
Block::search(float const &x, float const &y, float const &z, int const &state, bool const &)
{
	if (this->_cube.vertexInside(x, y, z) && this->_state == state)
		return (this);
	return (NULL);
}

Octree *
Block::insert(float const &, float const &, float const &, uint32_t const &, int32_t const &state)
{
	this->setState(state);
	return (this);
}

void
Block::render(Core &core) const
{
	(void)core;
}

void
Block::renderRidges(Core &core) const
{
	(void)core;
	glBindVertexArray(core.selectionVao);
	core.ms.push();
	core.ms.translate(_cube.getX(), _cube.getY(), _cube.getZ());
	core.ms.scale(_cube.getS(), _cube.getS(), _cube.getS());
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
