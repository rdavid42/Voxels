
#include "Plane.hpp"

Plane::Plane(void)
{
	return ;
}

Plane::~Plane(void)
{
	return ;
}

void
Plane::set(Vec3<float> &p1, Vec3<float> &p2, Vec3<float> &p3)
{
	Vec3<float>		aux1;
	Vec3<float>		aux2;

	aux1 = p1 - p2;
	aux2 = p3 - p2;

	normal.crossProduct(aux2, aux1);
	normal.normalize();
	position.set(p2);
	d = -normal.dotProduct(position);
}

float
Plane::distance(Vec3<float> &point)
{
	return (d + normal.dotProduct(point));
}
