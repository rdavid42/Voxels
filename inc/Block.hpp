
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
	Block(void);
	Block(Octree *parent, int const &state, int const &type);
	Block(int const &type);
	Block(float const &x, float const &y, float const &z, float const &s);
	virtual ~Block(void);

	virtual Octree *		search(float const &x, float const &y, float const &z);
	virtual Octree *		search(float const &x, float const &y, float const &z, int const &state, bool const &allowOutside);
	virtual Octree *		insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, int const &type);
	virtual void			render(Core &core) const;
	virtual void			renderRidges(Core &core) const;
	virtual Cube			getCube(void) const;
	virtual int				getType(void) const;
	void					remove(void);
	Chunk *					getChunk(void);

	Block				&operator=(Block const &rhs);

private:
	Block(Block const &src);

	int						_type;
};

std::ostream			&operator<<(std::ostream &o, Block const &i);

#endif
