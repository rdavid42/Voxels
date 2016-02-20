
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
Chunk::generateMesh(void)
{
	float					x, y, z, bs;
	Vec3<float>				b[6]; // position of quads up/bottom/back/front/left/right
	Vec3<float>				s[6]; // size of quads
	float					cx, cy, cz; // chunk world coordinate
	Vec3<int>				i; // block coordinate
	float					bt; // block type
	int						lbt[6]; // last block type for each side of a voxel
	int						j; // reusable index

	cx = getCube().getX();
	cy = getCube().getY();
	cz = getCube().getZ();
	bs = BLOCK_SIZE;
	for (y = cy; y < cy + CHUNK_SIZE; y += BLOCK_SIZE)
	{
		for (x = cx; x < cx + CHUNK_SIZE; x += BLOCK_SIZE)
		{
			// init greedy voxels up and bottom
			for (j = 0; j < 2; ++j)
			{
				lbt[j] = AIR;
				s[j].set(bs, bs, 0.0f);
			}
			// --------------
			for (z = cz; z < cz + CHUNK_SIZE; z += BLOCK_SIZE)
			{
				// set current block index
				i.set((x - cx) / BLOCK_SIZE, (y - cy) / BLOCK_SIZE, (z - cz) / BLOCK_SIZE);
				// get current block type
				bt = getBlock(i.x, i.y, i.z).getType();
				for (j = 0; j < 2; ++j)
				{
					if (s[j].z > 0.0f && (lbt[j] != bt || lbt[j] == AIR))
					{
						mesh.pushQuad(j, b[j].x, b[j].y, b[j].z, s[j].x, s[j].y, s[j].z, getTextureIndex(j, lbt[j]));
						lbt[j] = AIR;
						s[j].z = 0.0f;
					}
				}
				if (bt != AIR)
				{
					// up/bottom quads init and generation
					for (j = 0; j < 2; ++j)
					{
						if (s[j].z == 0.0f && checkEmpty(j, i))
						{
							b[j].x = x;
							b[j].y = y;
							b[j].z = z;
							lbt[j] = bt;
							s[j].z = BLOCK_SIZE;
						}
						else if (lbt[j] == bt)
							s[j].z += BLOCK_SIZE;
						if (s[j].z > 0.0f && (lbt[j] != bt || lbt[j] == AIR || z >= cz + CHUNK_SIZE - BLOCK_SIZE))
						{
							mesh.pushQuad(j, b[j].x, b[j].y, b[j].z, bs, bs, s[j].z, getTextureIndex(j, lbt[j]));
							lbt[j] = AIR;
							s[j].z = 0.0f;
						}
					}
					// if ((i.y + 1 < CHUNK_SIZE && getBlock(i.x, i.y + 1, i.z).getType() == AIR) || i.y + 1 == CHUNK_SIZE) // Up
						// mesh.pushUpFace(x, y, z, bs, getTextureIndex(V_UP, bt));
					// if ((i.y - 1 >= 0 && getBlock(i.x, i.y - 1, i.z).getType() == AIR) || i.y - 1 < 0) // Bottom
						// mesh.pushBottomFace(x, y, z, bs, getTextureIndex(V_BOTTOM, bt));
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
