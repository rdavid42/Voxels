
#ifndef		LINK_HPP
# define	LINK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Octree.hpp"
# include "Core.hpp"

class Link : public Octree
{
protected:
	Octree				*_children[CHD_MAX];

public:
	Link(void);
	Link(float const &x, float const &y, float const &z, float const &s);
	virtual ~Link(void);

	void				createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s, int32_t const &state);
	virtual Octree *	insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state);
	virtual Octree *	search(float const &x, float const &y, float const &z);
	virtual Octree *	search(float const &x, float const &y, float const &z, int const &state);
	virtual void		render(Core &core) const;

	Octree *			getChild(uint32_t const &i);
	void				setChild(uint32_t const &i, Octree *const child);

	Link &				operator=(Link const &rhs);

private:
	Link(Link const &src);
};

std::ostream &			operator<<(std::ostream &o, Link const &i);

#endif
