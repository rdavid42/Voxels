
#ifndef		OCTREE_HPP
# define	OCTREE_HPP

# include <iostream>
# include "Cube.hpp"
# include "Constants.hpp"
# include "Vec3.hpp"

class Chunk;
class Core;

class Octree
{
protected:
	int32_t				_state;
	Cube				_cube;
	Octree				*_parent;

public:
	Octree(void);
	Octree(Cube const &c);
	Octree(float const &x, float const &y, float const &z, float const &s);
	Octree(Octree const &src);
	virtual ~Octree(void);

	virtual Octree *	insert(float const &, float const &, float const &, uint32_t const &, int32_t const &) { return (NULL); }
	virtual Octree *	search(float const &, float const &, float const &) { return (NULL); }
	virtual Octree *	search(float const &, float const &, float const &, int const &) { return (NULL); }
	virtual void		render(Core &core) const { (void)core; }
	virtual void		renderRidges(Core &core) const { (void)core; }
	virtual void		deleteChild(Octree *child) { (void)child; }
	virtual Chunk *		getChunk(void) { return (NULL); }
	void				setCube(float const &x, float const &y, float const &z, float const &s);
	void				setState(int32_t const &state);
	void				setParent(Octree *parent);
	Cube const &		getCube(void) const;
	Cube *				getCube(void);
	int32_t const &		getState(void) const;
	Octree *			getParent(void);
	uint32_t			getDepth(void);
	virtual Octree *	getChild(uint32_t const &) { return (NULL); }
	virtual void		setChild(uint32_t const &, Octree *const) {}

	Octree				&operator=(Octree const &rhs);
};

std::ostream			&operator<<(std::ostream &o, Octree const &i);

#endif
