
#include "Core.hpp"
#include "Chunk.hpp"

Chunk::Chunk(void) : Link(), generated(false)
{
	_state = CHUNK;
	return ;
}

Chunk::Chunk(float const &x, float const &y, float const &z, float const &s) : Link(x, y, z, s), generated(false)
{
	_state = CHUNK;
	return ;
}

Chunk::~Chunk(void)
{
	if (glIsBuffer(vbo))
		glDeleteBuffers(1, &vbo);
	if (glIsVertexArray(vao))
		glDeleteVertexArrays(1, &vao);
	for (int i = 0; i < CHD_MAX; ++i)
	{
		if (_children[i] != NULL)
			delete _children[i];
		_children[i] = NULL;
	}
}

void
Chunk::deleteChild(Octree *child)
{
	std::cerr << "hip" << std::endl;
	for (int i = 0; i < CHD_MAX; ++i)
	{
		if (_children[i] == child)
		{
			delete _children[i];
			_children[i] = NULL;
		}
	}
}

Chunk *
Chunk::getChunk(void)
{
	return (this);
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
Chunk::renderLines(Core &core) const
{
	if (renderDone)
	{
		if (meshSize > 0)
		{
			glBindVertexArray(vao);
			core.ms.push();
				glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
				glDrawArrays(GL_LINES, 0, meshSize);
			core.ms.pop();
		}
	}
}

void
Chunk::renderRidges(Core &core) const
{
	(void)core;
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
