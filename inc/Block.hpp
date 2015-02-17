
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
	Block(Block const &src);
	~Block(void);

	Block				&operator=(Block const &rhs);
};

std::ostream				&operator<<(std::ostream &o, Block const &i);

#endif
