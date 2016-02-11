
#include "Core.hpp"
#include "Chunk.hpp"

Chunk::Chunk(void) : Link(),
			_generating(false), _generated(false), _renderDone(false), _stopGenerating(false), _removable(false), vao(0), vbo(0)
{
	_state = CHUNK;
	return ;
}

Chunk::Chunk(float const &x, float const &y, float const &z, float const &s) : Link(x, y, z, s),
			_generating(false), _generated(false), _renderDone(false), _stopGenerating(false), _removable(false), vao(0), vbo(0)
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
	if (mesh.size() > 0)
		mesh.clear();
	vao = 0;
	vbo = 0;
	_generating = false;
	_generated = false;
	_renderDone = false;
	_stopGenerating = false;
	_removable = false;
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
	if (_renderDone)
	{
		if (mesh.vertices() > 0)
		{
			glBindVertexArray(vao);
			core.ms.push();
				glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
				glDrawArrays(GL_TRIANGLES, 0, mesh.vertices());
			core.ms.pop();
		}
	}
}

void
Chunk::renderLines(Core &core) const
{
	if (_renderDone)
	{
		if (mesh.vertices() > 0)
		{
			glBindVertexArray(vao);
			core.ms.push();
				glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
				glDrawArrays(GL_LINES, 0, mesh.vertices());
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

bool const &
Chunk::getGenerating(void)
{
	return (_generating);
}

bool const &
Chunk::getGenerated(void)
{
	return (_generated);
}

bool const &
Chunk::getRenderDone(void)
{
	return (_renderDone);
}

bool const &
Chunk::getStopGenerating(void)
{
	return (_stopGenerating);
}

bool const &
Chunk::getRemovable(void)
{
	return (_removable);
}

void
Chunk::setGenerating(bool const &val)
{
	_generating = val;
}

void
Chunk::setGenerated(bool const &val)
{
	_generated = val;
}

void
Chunk::setRenderDone(bool const &val)
{
	_renderDone = val;
}

void
Chunk::setStopGenerating(bool const &val)
{
	_stopGenerating = val;
}

void
Chunk::setRemovable(bool const &val)
{
	_removable = val;
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
