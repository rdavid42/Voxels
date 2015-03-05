
#ifndef		PARTICLEENGINE_HPP
# define	PARTICLEENGINE_HPP

# include <iostream>
# include <list>
# include <Vec3.hpp>
# include <particle.hpp>

class particleEngine
{
	public:
		particleEngine(void);
		particleEngine(particleEngine const & src);
		~particleEngine(void);

		std::list<Particle *>	particleList;

		void		blockDestruction(Vec3<float> pos, Vec3<float> color);
		void		cleanDead(void);
		void		particleGestion(void);

		particleEngine & operator = (particleEngine const & rhs);

	private:
};

std::ostream			&operator<<(std::ostream &o, particleEngine const &i);

#endif
