
#ifndef		BLOCK_HPP
# define	BLOCK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Octree.hpp"
# include "Chunk.hpp"

class Core;

class Block : public Octree
{
public:
	int					type;

	Block(void);
	Block(int const &t);
	Block(float const &x, float const &y, float const &z, float const &s);
	virtual ~Block(void);

	virtual Octree *		search(float const &x, float const &y, float const &z);
	virtual Octree *		search(float const &x, float const &y, float const &z, int const &state, bool const &allowOutside);
	virtual Octree *		insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, int const &type);
	virtual void			render(Core &core) const;
	virtual void			renderRidges(Core &core) const;
	virtual Cube			getCube(void) const;
	void					remove(void);
	Chunk *					getChunk(void);

	Block				&operator=(Block const &rhs);

private:
	Block(Block const &src);
};

std::ostream			&operator<<(std::ostream &o, Block const &i);

#endif
