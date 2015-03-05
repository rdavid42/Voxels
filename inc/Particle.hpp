
#ifndef		PARTICLE_HPP
# define	PARTICLE_HPP

# include <iostream>
# include <math.h>
# include <Constants.hpp>
# include <Vec3.hpp>

class Particle
{
	public:
		Particle(void);
		Particle(Particle const & src);
		~Particle(void);

		void			run(void);
		void			drawTriangle(void);
		void			initwithPos(float x, float y, float z);
		void			initForDestruction(Vec3<float> &pos, Vec3<float> &color);
		void			update(void);
		bool			isDead(void);
		Vec3<float>		getPosition(void) const;
		void			setPosition(Vec3<float> &newPos);
		Vec3<float>		getVelocity(void) const;
		void			setVelocity(Vec3<float> &newVelocity);
		Vec3<float>		getAcceleration(void) const;
		void			setAcceleration(Vec3<float> &newAcceleration);
		float			getLifeSpan(void) const;
		void			setLifeSpan(int &newLifeSpan);

		Particle & operator = (Particle const & rhs);

	private :
		Vec3<float>		_color;
		Vec3<float>		_endColor;
		Vec3<float>		_deltaColor;
		Vec3<float>		_position;
		Vec3<float>		_velocity;
		Vec3<float>		_acceleration;
		float			_lifeSpan;


};

std::ostream			&operator<<(std::ostream &o, Particle const &i);

#endif
