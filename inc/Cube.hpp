
#ifndef		CUBE_HPP
# define	CUBE_HPP

# include <iostream>
# include <stdint.h>

class Cube 
{
private:
	float	_x;
	float	_y;
	float	_z;
	float	_s;

public:
	Cube(void);
	Cube(float const &x, float const &y, float const &z, float const &s);
	Cube(Cube const &src);
	virtual ~Cube(void);

	bool				vertexInside(float const &x, float const &y, float const &z);

	float const			&getX(void) const;
	float const			&getY(void) const;
	float const			&getZ(void) const;
	float const			&getS(void) const;
	void				setX(float const &x);
	void				setY(float const &y);
	void				setZ(float const &z);
	void				setS(float const &s);

	Cube				&operator=(Cube const &rhs);
};

std::ostream				&operator<<(std::ostream &o, Cube const &i);

#endif
