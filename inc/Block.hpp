
#ifndef		BLOCK_HPP
# define	BLOCK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Struct.hpp"
# include "Octree.hpp"

class Block : public Octree
{
public:
	int					n;
	Triangle<float>		t[5];

	Block(void);
	Block(Vec3<float> const &c, float const &x, float const &y, float const &z, float const &s);
	Block(Block const &src);
	virtual ~Block(void);

	virtual Octree *	search(float const &x, float const &y, float const &z);
	virtual Octree *	search(float const &x, float const &y, float const &z, int const &state);
	virtual Octree *	insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, Vec3<float> const &c, bool const &simplify);

	virtual void		render(void) const;

	Block				&operator=(Block const &rhs);
};

std::ostream			&operator<<(std::ostream &o, Block const &i);

#endif
