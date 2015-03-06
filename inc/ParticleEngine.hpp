
#ifndef		PARTICLEENGINE_HPP
# define	PARTICLEENGINE_HPP

# include <iostream>
# include <list>
# include <Vec3.hpp>
# include <Particle.hpp>

class ParticleEngine
{
	public:
		ParticleEngine(void);
		ParticleEngine(ParticleEngine const & src);
		~ParticleEngine(void);

		std::list<Particle *>	particleList;

		void		blockDestruction(Vec3<float> pos, Vec3<float> color);
		void		cleanDead(void);
		void		particleGestion(void);

		ParticleEngine & operator = (ParticleEngine const & rhs);

	private:
};

std::ostream			&operator<<(std::ostream &o, ParticleEngine const &i);

#endif
