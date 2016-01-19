#ifndef OCTREE_HPP
# define OCTREE_HPP

# include "Vec3.hpp"
# include "Cube.hpp"

// number of children
# define CHD_MAX					8

// Octree states
# define EMPTY						0b00000000
# define GROUND						0b00000001
# define BLOCK						0b00000010
# define CHUNK						0b00000100

class Octree
{
protected:
	int32_t				_state;
	Cube				_cube;
	Octree				*_parent;

public:
	Vec3<float>			c;

	Octree();
	Octree(float const &x, float const &y, float const &z, float const &s);
	Octree(Vec3<float> const &c, float const &x, float const &y, float const &z, float const &s);
	virtual ~Octree();

	void				remove(void);
	virtual Octree *	search(float const &, float const &, float const &) { return (NULL); }
	virtual Octree *	search(float const &, float const &, float const &, int const &) { return (NULL); }
	virtual void		render(void) const {}
	void				setCube(float const &x, float const &y, float const &z, float const &s);
	void				setState(int32_t const &state);
	void				setParent(Octree *parent);
	Cube const &		getCube(void) const;
	Cube *				getCube(void);
	int32_t const &		getState(void) const;
	Octree *			getParent(void);
	virtual Octree *	getChild(uint32_t const &) { return (NULL); }
	virtual void		setChild(uint32_t const &, Octree *const) {}
	uint32_t			getDepth(void);
};

std::ostream			&operator<<(std::ostream &o, Octree const &i);

#endif
