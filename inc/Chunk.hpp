
#ifndef		CHUNK_HPP
# define	CHUNK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Octree.hpp"

class Chunk : public Octree
{
public:
	float			**hm; // heightmap

	Chunk(void);
	Chunk(float const &x, float const &y, float const &z, float const &s);
	Chunk(Chunk const &src);
	virtual ~Chunk(void);

	virtual	void		render(void) const;

	Chunk				&operator=(Chunk const &rhs);
};

std::ostream				&operator<<(std::ostream &o, Chunk const &i);

#endif
