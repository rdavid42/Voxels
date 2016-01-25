
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
		if (meshSize > 0)
		{
			glBindVertexArray(vao);
			core.ms.push();
				glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
				glDrawArrays(GL_TRIANGLES, 0, meshSize);
			core.ms.pop();
		}
	}
}

void
Chunk::renderRidges(Core &core) const
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
