
#ifndef		BLOCK_HPP
# define	BLOCK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Octree.hpp"

class Block : public Octree
{
public:
	Block(void);
	Block(float const &x, float const &y, float const &z, float const &s);
	virtual ~Block(void);

	virtual Octree *	search(float const &x, float const &y, float const &z);
	virtual Octree *	search(float const &x, float const &y, float const &z, int const &state);
	virtual Octree *	insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state);

	virtual void		render(Core &core) const;

	Block				&operator=(Block const &rhs);

private:
	Block(Block const &src);
};

std::ostream			&operator<<(std::ostream &o, Block const &i);

#endif
