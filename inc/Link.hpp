
#ifndef		LINK_HPP
# define	LINK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Struct.hpp"
# include "Octree.hpp"

class Link : public Octree
{
protected:
	Vec3<float>			c;
	Octree				*_children[CHD_MAX];

public:
	Link(void);
	Link(Vec3<float> const &c, float const &x, float const &y, float const &z, float const &s);
	Link(float const &x, float const &y, float const &z, float const &s);
	Link(Link const &src);
	virtual ~Link(void);

	void				createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s, int32_t const &state, Vec3<float> const &c);
	virtual Octree *	insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, Vec3<float> const &c);
	virtual Octree *	search(float const &x, float const &y, float const &z);
	virtual Octree *	search(float const &x, float const &y, float const &z, int const &state);
	virtual void		render(void) const;

	Octree *			getChild(uint32_t const &i);
	void				setChild(uint32_t const &i, Octree *const child);

	Link &				operator=(Link const &rhs);
};

std::ostream &			operator<<(std::ostream &o, Link const &i);

#endif
