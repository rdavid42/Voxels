#ifndef PLANE_HPP
# define PLANE_HPP

# include "Vec3.hpp"

class Plane
{
public:
	Vec3<float>		normal;
	Vec3<float>		position;
	float			d;

	Plane(void);
	~Plane(void);

	void			set(Vec3<float> &p1, Vec3<float> &p2, Vec3<float> &p3);
	float			distance(Vec3<float> const &point) const;

private:
};

#endif