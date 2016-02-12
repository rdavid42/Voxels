
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
	Octree *			_parent;
	int16_t				_state;

public:
	Octree(void);
	Octree(Octree *parent, int16_t const &state);
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
	virtual void				backwardSimplification(void) {};
	virtual int16_t				getType(void) const { return (NONE); };
	void						setState(int16_t const &state);
	void						setParent(Octree *parent);
	int16_t const &				getState(void) const;
	Octree *					getParent(void) const;
	uint32_t					getDepth(void);
	virtual Octree *			getChild(uint32_t const &) { return (NULL); }
	virtual void				setChild(uint32_t const &, Octree *const) {}

	Octree				&operator=(Octree const &rhs);
};

std::ostream			&operator<<(std::ostream &o, Octree const &i);

#endif
