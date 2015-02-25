
#ifndef		BLOCK_HPP
# define	BLOCK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Struct.hpp"
# include "Octree.hpp"

class Block : public Octree
{
public:
	Vec3<float>			c;

	Block(void);
	Block(Vec3<float> const &color);
	Block(float const &x, float const &y, float const &z, float const &s);
	Block(Block const &src);
	virtual ~Block(void);

	virtual void		render(void) const;

	Block				&operator=(Block const &rhs);
};

std::ostream				&operator<<(std::ostream &o, Block const &i);

#endif
