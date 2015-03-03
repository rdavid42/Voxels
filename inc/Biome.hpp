
#ifndef		BIOME_HPP
# define	BIOME_HPP

# include <iostream>
# include "Constants.hpp"
# include "Link.hpp"

class Biome : public Link
{
public:
	Biome(void);
	Biome(float const &x, float const &y, float const &z, float const &s);
	virtual ~Biome(void);

	virtual void		render(void) const {}

	Biome				&operator=(Biome const &rhs);

private:
	Biome(Biome const &src);
};

std::ostream				&operator<<(std::ostream &o, Biome const &i);

#endif
