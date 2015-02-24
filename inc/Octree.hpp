
#ifndef		OCTREE_HPP
# define	OCTREE_HPP

# include <iostream>
# include "Cube.hpp"
# include "Camera.hpp"
# include "Constants.hpp"
# include "Vec3.hpp"
# include "Triangle.hpp"

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

	void				remove(void);
	virtual void		render(void) const;
	void				drawCube(float const &x, float const &y, float const &z, float const &s) const;
	void				drawCubeRidges(float const &x, float const &y, float const &z, float const &s) const;
	void				setCube(float const &x, float const &y, float const &z, float const &s);
	void				setState(int32_t const &state);
	void				setParent(Octree *parent);
	Cube const &		getCube(void) const;
	Cube *				getCube(void);
	int32_t const &		getState(void) const;
	Octree *			getParent(void);
	uint32_t			getDepth(void);

	Octree				&operator=(Octree const &rhs);
};

std::ostream			&operator<<(std::ostream &o, Octree const &i);

#endif
