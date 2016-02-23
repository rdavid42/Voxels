
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

inline float
getTextureIndex(int const &voxelFace, int const &blockType)
{
	if (voxelFace == Chunk::V_BOTTOM)
	{
		if (blockType == GRASS)
			return (DIRT - 1);
	}
	else if (voxelFace > Chunk::V_BOTTOM) // sides
	{
		if (blockType == GRASS)
			return (SIDE_GRASS - 1);
	}
	return (blockType - 1);
}

inline bool
Chunk::checkEmpty(int const &side, Vec3<int> const &p) const
{
	if (side == V_UP)
		return ((p.y + 1 < CHUNK_SIZE && getBlock(p.x, p.y + 1, p.z).getType() == AIR) || p.y + 1 == CHUNK_SIZE);
	else if (side == V_BOTTOM)
		return ((p.y - 1 >= 0 && getBlock(p.x, p.y - 1, p.z).getType() == AIR) || p.y - 1 < 0);
	else if (side == V_BACK)
		return ((p.z - 1 >= 0 && getBlock(p.x, p.y, p.z - 1).getType() == AIR) || p.z - 1 < 0);
	else if (side == V_FRONT)
		return ((p.z + 1 < CHUNK_SIZE && getBlock(p.x, p.y, p.z + 1).getType() == AIR) || p.z + 1 == CHUNK_SIZE);
	else if (side == V_LEFT)
		return ((p.x - 1 >= 0 && getBlock(p.x - 1, p.y, p.z).getType() == AIR) || p.x - 1 < 0);
	else if (side == V_RIGHT)
		return ((p.x + 1 < CHUNK_SIZE && getBlock(p.x + 1, p.y, p.z).getType() == AIR) || p.x + 1 == CHUNK_SIZE);
	return (false);
}

void
Chunk::generateGreedyMesh(void)
{
	float					x, y, z, bs;
	Vec3<float>				c; // chunk world position
	Vec3<int>				i; // block coordinate
	float					bt; // current block type
	int						j, k, l;
	char					hq[2][CHUNK_SIZE][CHUNK_SIZE][3]; // [up/bottom][x][z][width(z)/height(x)/type]
	int						qz[2]; // current quad z (up/bottom)

	c.set(_cube.getX(), _cube.getY(), _cube.getZ());
	bs = BLOCK_SIZE;
	for (y = c.y; y < c.y + CHUNK_SIZE; y += BLOCK_SIZE)
	{
		for (j = 0; j < 2; ++j)
		{
			for (k = 0; k < CHUNK_SIZE; ++k)
			{
				for (l = 0; l < CHUNK_SIZE; ++l)
				{
					hq[j][k][l][0] = 0;
					hq[j][k][l][1] = 1;
					hq[j][k][l][2] = AIR;
				}
			}
		}
		for (x = c.x; x < c.x + CHUNK_SIZE; x += BLOCK_SIZE)
		{
			for (j = 0; j < 2; ++j)
				qz[j] = -1;
			for (z = c.z; z < c.z + CHUNK_SIZE; z += BLOCK_SIZE)
			{
				i.set((x - c.x) / BLOCK_SIZE, (y - c.y) / BLOCK_SIZE, (z - c.z) / BLOCK_SIZE);
				bt = getBlock(i.x, i.y, i.z).getType();
				if (bt != AIR)
				{
					for (j = 0; j < 2; ++j)
					{
						if (checkEmpty(j, i))
						{
							if (qz[j] == -1)
							{
								qz[j] = i.z;
								hq[j][i.x][qz[j]][2] = bt;
							}
							if (bt == hq[j][i.x][qz[j]][2])
								hq[j][i.x][qz[j]][0]++;
							else
							{
								qz[j] = i.z;
								hq[j][i.x][qz[j]][2] = bt;
								hq[j][i.x][qz[j]][0]++;
							}
						}
						else
							qz[j] = -1;
					}
					if (checkEmpty(V_BACK, i)) // Back
						mesh.pushBackFace(x, y, z, bs, getTextureIndex(V_BACK, bt));
					if (checkEmpty(V_FRONT, i)) // Front
						mesh.pushFrontFace(x, y, z, bs, getTextureIndex(V_FRONT, bt));
					if (checkEmpty(V_LEFT, i)) // Left
						mesh.pushLeftFace(x, y, z, bs, getTextureIndex(V_LEFT, bt));
					if (checkEmpty(V_RIGHT, i)) // Right
						mesh.pushRightFace(x, y, z, bs, getTextureIndex(V_RIGHT, bt));
				}
				else
				{
					for (j = 0; j < 2; ++j)
						qz[j] = -1;
				}
			}
		}
		// merge horizontal quads
		for (j = 0; j < 2; ++j)
		{
			for (k = CHUNK_SIZE - 1; k > 0; --k) // x, merge backwards, skip first row
			{
				for (l = 0; l < CHUNK_SIZE; ++l) // z
				{
					if (hq[j][k][l][2] != AIR)
					{
						if (hq[j][k][l][2] == hq[j][k - 1][l][2]) // same quad type
						{
							if (hq[j][k][l][0] == hq[j][k - 1][l][0]) // same quad size
							{
								hq[j][k - 1][l][1] += hq[j][k][l][1];
								hq[j][k][l][0] = 0;
								hq[j][k][l][1] = 1;
								hq[j][k][l][2] = AIR;
							}
						}
					}
				}
			}
		}
		// push horizontal quads
		for (j = 0; j < 2; ++j)
			for (k = 0; k < CHUNK_SIZE; ++k) // x
				for (l = 0; l < CHUNK_SIZE; ++l) // z
					if (hq[j][k][l][2] != AIR)
						mesh.pushQuad(j, c.x + k * bs, y, c.z + l * bs,
									hq[j][k][l][1] * BLOCK_SIZE, bs, hq[j][k][l][0] * BLOCK_SIZE,
									getTextureIndex(j, hq[j][k][l][2]));
	}
	// merge vertical quads
	// push vertical quads
}

void
Chunk::generateNaiveMesh(void)
{
	float					x, y, z, bs;
	float					cx, cy, cz; // chunk world coordinate
	Vec3<int>				i; // block coordinate
	float					bt; // block type

	cx = getCube().getX();
	cy = getCube().getY();
	cz = getCube().getZ();
	bs = BLOCK_SIZE;
	for (y = cy; y < cy + CHUNK_SIZE; y += BLOCK_SIZE)
	{
		for (x = cx; x < cx + CHUNK_SIZE; x += BLOCK_SIZE)
		{
			for (z = cz; z < cz + CHUNK_SIZE; z += BLOCK_SIZE)
			{
				// set current block index
				i.set((x - cx) / BLOCK_SIZE, (y - cy) / BLOCK_SIZE, (z - cz) / BLOCK_SIZE);
				// get current block type
				bt = getBlock(i.x, i.y, i.z).getType();
				if (bt != AIR)
				{
					if (checkEmpty(V_UP, i))
						mesh.pushUpFace(x, y, z, bs, getTextureIndex(V_UP, bt));
					if (checkEmpty(V_BOTTOM, i))
						mesh.pushBottomFace(x, y, z, bs, getTextureIndex(V_BOTTOM, bt));
					if (checkEmpty(V_BACK, i)) // Back
						mesh.pushBackFace(x, y, z, bs, getTextureIndex(V_BACK, bt));
					if (checkEmpty(V_FRONT, i)) // Front
						mesh.pushFrontFace(x, y, z, bs, getTextureIndex(V_FRONT, bt));
					if (checkEmpty(V_LEFT, i)) // Left
						mesh.pushLeftFace(x, y, z, bs, getTextureIndex(V_LEFT, bt));
					if (checkEmpty(V_RIGHT, i)) // Right
						mesh.pushRightFace(x, y, z, bs, getTextureIndex(V_RIGHT, bt));
				}
			}
		}
	}
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
