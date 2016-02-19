
#include "Chunk.hpp"
#include "Core.hpp"

Chunk::Chunk(void)
{
	init();
	return ;
}

Chunk::Chunk(float const &x, float const &y, float const &z, float const &s)
			
{
	init();
	_cube = Cube(x, y, z, s);
	return ;
}

Chunk::~Chunk(void)
{
	_generating = false;
	_generated = false;
	_renderDone = false;
	_stopGenerating = false;
	_removable = false;
	deleteBlocks();
}

void
Chunk::deleteBlocks(void)
{
	if (_blocks != 0)
	{
		delete [] _blocks;
		_blocks = 0;
	}
}

void
Chunk::init(void)
{
	_generating = false;
	_generated = false;
	_renderDone = false;
	_stopGenerating = false;
	_removable = false;
	_blocks = new Block[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
}

void
Chunk::render(Core &core) const
{
	(void)core;
	if (_renderDone)
	{
		if (mesh.getVertices() > 0)
		{
			glBindVertexArray(mesh.getVAO());
			core.ms.push();
				glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
				glDrawArrays(mesh.getMode(), 0, mesh.getVertices());
			core.ms.pop();
		}
	}
}

void
Chunk::renderLines(Core &core) const
{
	if (_renderDone)
	{
		if (mesh.getVertices() > 0)
		{
			glBindVertexArray(mesh.getVAO());
			core.ms.push();
				glUniformMatrix4fv(core.objLoc, 1, GL_FALSE, core.ms.top().val);
				glDrawArrays(GL_LINES, 0, mesh.getVertices());
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

void
Chunk::setBlock(int const &x, int const &y, int const &z, uint8_t const &type)
{
	_blocks[(x * CHUNK_SIZE + y) * CHUNK_SIZE + z].setType(type);
}

Block const &
Chunk::getBlock(int const &x, int const &y, int const &z) const
{
	return (_blocks[(x * CHUNK_SIZE + y) * CHUNK_SIZE + z]);
}

Block const *
Chunk::getBlocks(void) const
{
	return (_blocks);
}

Cube const &
Chunk::getCube(void) const
{
	return (_cube);
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
