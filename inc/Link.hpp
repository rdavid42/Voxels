
#ifndef		LINK_HPP
# define	LINK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Octree.hpp"

class Core;

class Link : public Octree
{
protected:
	Cube				_cube;
	Octree *			_children[CHD_MAX];

public:
	Link(void);
	Link(float const &x, float const &y, float const &z, float const &s);
	virtual ~Link(void);

	virtual Octree *	insert(float const &x, float const &y, float const &z, uint32_t const &depth, int32_t const &state, int const &type);
	virtual Octree *	search(float const &x, float const &y, float const &z);
	virtual Octree *	search(float const &x, float const &y, float const &z, int const &state, bool const &allowOutside);
	virtual void		render(Core &core) const;
	virtual void		renderLines(Core &core) const;
	virtual void		renderRidges(Core &core) const;
	virtual Chunk *		getChunk(void);
	virtual void		remove(void);
	virtual void		deleteChild(Octree *child);
	virtual void		backwardSimplification(void);
	void				createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s, int32_t const &state);

	void				setCube(float const &x, float const &y, float const &z, float const &s);
	Cube				getCube(void) const;
	Octree *			getChild(uint32_t const &i);
	void				setChild(uint32_t const &i, Octree *child);

	Link &				operator=(Link const &rhs);

private:
	Link(Link const &src);
};

std::ostream &			operator<<(std::ostream &o, Link const &i);

#endif
