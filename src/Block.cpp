
#include "Block.hpp"

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

Octree *
Block::search(float const &x, float const &y, float const &z)
{
	if (this->_cube.vertexInside(x, y, z))
		return (this);
	return (NULL);
}
// -------------------------------------------------------------------

Octree *
Block::search(float const &x, float const &y, float const &z, int const &state)
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
	core.ms.push();
		core.ms.translate(_cube.getX(), _cube.getY(), _cube.getZ());
		// std::cerr << _cube.getX() << "," << _cube.getY() << "," << _cube.getZ() << "," << _cube.getS() << std::endl;
		glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
		glBindTexture(GL_TEXTURE_2D, core.tex[1]);
		glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, (void *)(sizeof(GLushort) * 0));
		glBindTexture(GL_TEXTURE_2D, core.tex[0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)(sizeof(GLushort) * 24));
		glBindTexture(GL_TEXTURE_2D, core.tex[2]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)(sizeof(GLushort) * 30));
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
