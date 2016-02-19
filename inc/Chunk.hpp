
#ifndef CHUNK_HPP
# define CHUNK_HPP

# include <iostream>
# include <vector>
# include "Constants.hpp"
# include "Cube.hpp"
# include "Mesh.hpp"
# include "Block.hpp"
# include "Vec3.hpp"

class Chunk
{
private:
	bool							_generating;
	bool							_generated;
	bool							_renderDone;
	bool							_stopGenerating;
	bool							_removable;
	Vec3<float>						_position;
	Block							***_blocks;

public:
	Mesh							mesh;

	Chunk(void);
	Chunk(float const &x, float const &y, float const &z);
	~Chunk(void);

	void							init(void);

	void							render(Core &core) const;
	void							renderLines(Core &core) const;
	void							renderRidges(Core &core) const;

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
	Block const &					getBlock(int const &x, int const &y, int const &z);

	Vec3<float> const &				getCube(void) const;

	Chunk							&operator=(Chunk const &rhs);

private:
	Chunk(Chunk const &src);
};

std::ostream						&operator<<(std::ostream &o, Chunk const &i);

#endif
