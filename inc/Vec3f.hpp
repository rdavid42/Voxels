
#ifndef		VEC3F_HPP
# define	VEC3F_HPP

# include <iostream>
# include <math.h>

class Vec3f
{
public:
	float				x;
	float				y;
	float				z;

	Vec3f(void);
	Vec3f(float const &x, float const &y, float const &z);
	~Vec3f(void);

	Vec3f				crossProduct(Vec3f const &v) const;
	void				normalize(void);

	Vec3f				&operator=(Vec3f const &rhs);
	Vec3f				operator+(Vec3f const &rhs);
	Vec3f				operator*(Vec3f const &rhs);
	Vec3f				operator*(float const &rhs);
	Vec3f				&operator+=(Vec3f const &rhs);
	Vec3f				&operator-=(Vec3f const &rhs);
private:
	Vec3f(Vec3f const &src);
};

std::ostream				&operator<<(std::ostream &o, Vec3f const &i);

#endif
