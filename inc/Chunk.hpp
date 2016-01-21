
#ifndef CHUNK_HPP
# define CHUNK_HPP

# include <iostream>
# include "Constants.hpp"
# include "Link.hpp"

class Chunk : public Link
{
public:
	bool			generated;
	Vec3<uint8_t>	pos;

	Chunk(void);
	Chunk(float const &x, float const &y, float const &z, float const &s);
	virtual ~Chunk(void);

	virtual	void		render(Core &core) const;

	Chunk				&operator=(Chunk const &rhs);

private:
	Chunk(Chunk const &src);
};

std::ostream				&operator<<(std::ostream &o, Chunk const &i);

#endif
