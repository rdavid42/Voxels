
#ifndef		OCTREE_HPP
# define	OCTREE_HPP

# include <iostream>
# include "Constants.hpp"
# include "Cube.hpp"

class Chunk;
class Core;

class Octree
{
protected:
	int32_t				_state;
	Octree *			_parent;

public:
	Octree(void);
	Octree(Octree const &src);
	virtual ~Octree(void);

	virtual Octree *			insert(float const &, float const &, float const &, uint32_t const &, int32_t const &, int const &) { return (NULL); }
	virtual Octree *			search(float const &, float const &, float const &) { return (NULL); }
	virtual Octree *			search(float const &, float const &, float const &, int const &, bool const &) { return (NULL); }
	virtual void				render(Core &core) const { (void)core; }
	virtual void				renderLines(Core &core) const { (void)core; }
	virtual void				renderRidges(Core &core) const { (void)core; }
	virtual void				deleteChild(Octree *child) { (void)child; }
	virtual Chunk *				getChunk(void) { return (NULL); }
	virtual void				remove(void) {}

	virtual void				setCube(float const &, float const &, float const &, float const &) {};
	virtual Cube				getCube(void) const = 0;
	void						setState(int32_t const &state);
	void						setParent(Octree *parent);
	int32_t const &				getState(void) const;
	Octree *					getParent(void) const;
	uint32_t					getDepth(void);
	virtual Octree *			getChild(uint32_t const &) { return (NULL); }
	virtual void				setChild(uint32_t const &, Octree *const) {}

	Octree				&operator=(Octree const &rhs);
};

std::ostream			&operator<<(std::ostream &o, Octree const &i);

#endif
