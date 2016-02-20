
#ifndef CHUNK_HPP
# define CHUNK_HPP

# include <iostream>
# include <vector>
# include "Constants.hpp"
# include "Cube.hpp"
# include "Mesh.hpp"
# include "Block.hpp"

class Chunk
{
private:
	Block							*_blocks;
	bool							_generating;
	bool							_generated;
	bool							_renderDone;
	bool							_stopGenerating;
	bool							_removable;
	Cube							_cube;

public:
	Vec3<uint8_t>					pos; // position relative to other chunks
	Mesh							mesh;

	typedef enum					e_voxelFace
	{
		V_UP,
		V_BOTTOM,
		V_BACK,
		V_FRONT,
		V_LEFT,
		V_RIGHT
	}								voxelFace;

	Chunk(void);
	Chunk(float const &x, float const &y, float const &z, float const &s);
	~Chunk(void);

	void							init(void);
	void							deleteBlocks(void);

	void							render(Core &core) const;
	void							renderLines(Core &core) const;
	void							renderRidges(Core &core) const;

	bool							checkEmpty(int const &side, Vec3<int> const &p) const;
	void							generateMesh(void);

	bool const &					getGenerating(void);
	bool const &					getGenerated(void);
	bool const &					getRenderDone(void);
	bool const &					getStopGenerating(void);
	bool const &					getRemovable(void);

	void							setGenerating(bool const &val);
	void							setGenerated(bool const &val);
	void							setRenderDone(bool const &val);
	void							setStopGenerating(bool const &val);
	void							setRemovable(bool const &val);

	void							setBlock(int const &x, int const &y, int const &z, uint8_t const &type);
	Block const &					getBlock(int const &x, int const &y, int const &z) const;

	Cube const &					getCube(void) const;

	Block const *					getBlocks(void) const;

	Chunk							&operator=(Chunk const &rhs);

private:
	Chunk(Chunk const &src);
};

std::ostream						&operator<<(std::ostream &o, Chunk const &i);

#endif
