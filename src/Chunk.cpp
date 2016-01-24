
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
Chunk::createMesh(void)
{

}

void
Chunk::render(Core &core) const
{
	(void)core;
	if (renderDone)
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		core.ms.push();
			glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
			glDrawArrays(GL_TRIANGLES, 0, meshSize);
		core.ms.pop();
	}
	int			i;

	for (i = 0; i < CHD_MAX; ++i)
	{
		if (this->_children[i] != NULL)
			this->_children[i]->render(core);
	}
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
