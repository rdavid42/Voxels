
#ifndef		OCTREE_HPP
# define	OCTREE_HPP

# include <iostream>
# include "Cube.hpp"
# include "Camera.hpp"
# include "Constants.hpp"
# include "Vec3.hpp"

# define CHD_MAX		8

class Octree
{
private:
	int32_t				_state;
	Cube				_cube;
	Octree				*_parent;
	Octree				*_children[CHD_MAX];

public:
	Vec3<float>			c;
	bool				generated;
	bool				iterated;
	static uint32_t		block_depth;
	static uint32_t		chunk_depth;

	Octree(void);
	Octree(Cube const &c);
	Octree(float const &x, float const &y, float const &z, float const &s);
	Octree(Octree const &src);
	virtual ~Octree(void);

	int					subdivide(void);
	Octree *			getNeighbor(void);
	int					createChild(uint32_t const &i);
	void				createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s);
	void				createChild(uint32_t const &i, float const &x, float const &y, float const &z, float const &s, uint32_t const &state);
	void				grow(uint32_t const &i);
	Octree *			insert(float const &x, float const &y, float const &z, uint32_t const &depth, uint32_t const &state, Vec3<float> const &c);
	Octree *			search(float const &x, float const &y, float const &z);
	Octree *			search(float const &x, float const &y, float const &z, int const &state);
	void				remove(void);
	void				renderGround(void) const;
	void				drawCube(float const &x, float const &y, float const &z, float const &s) const;
	void				drawCubeRidges(float const &x, float const &y, float const &z, float const &s) const;
	void				setCube(float const &x, float const &y, float const &z, float const &s);
	void				setState(int32_t const &state);
	void				setParent(Octree *parent);
	void				setChild(uint32_t const &i, Octree *const child);
	Cube const &		getCube(void) const;
	Cube *				getCube(void);
	int32_t const &		getState(void) const;
	Octree *			getChild(uint32_t const &i) const;
	Octree *			getParent(void);
	uint32_t			getDepth(void);

	Octree				&operator=(Octree const &rhs);
};

std::ostream			&operator<<(std::ostream &o, Octree const &i);

#endif
