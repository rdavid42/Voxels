
#ifndef		VEC3_HPP
# define	VEC3_HPP

# include <iostream>
# include <math.h>

template<typename TYPE>
class Vec3
{
public:
	TYPE				x;
	TYPE				y;
	TYPE				z;

	Vec3(void) : x(0), y(0), z(0)
	{
		return ;
	}

	Vec3(TYPE const &x, TYPE const &y, TYPE const &z) : x(x), y(y), z(z)
	{
		return ;
	}

	Vec3(Vec3<TYPE> const &src) : x(src.x), y(src.y), z(src.z)
	{
		return ;
	}

	~Vec3(void)
	{
		return ;
	}

	Vec3<TYPE>
	crossProduct(Vec3<TYPE> const &v) const
	{
		Vec3<TYPE>	r;

		r.x = this->y * v.z - this->z * v.y;
		r.y = this->z * v.x - this->x * v.z;
		r.z = this->x * v.y - this->y * v.x;
		return (r);
	}

	void
	crossProduct(Vec3<TYPE> const &a, Vec3<TYPE> const &b)
	{
		this->x = a.y * b.z - b.y * a.z;
		this->y = a.z * b.x - b.z * a.x;
		this->z = a.x * b.y - b.x * a.y;
	}

	TYPE
	dotProduct(Vec3<TYPE> const &v) const
	{
		return (this->x * v.x + this->y * v.y + this->z * v.z);
	}

	void
	normalize(void)
	{
		TYPE const	h = sqrt(this->x * this->x + this->y * this->y + this->z * this->z);

		this->x = this->x / h;
		this->y = this->y / h;
		this->z = this->z / h;
	}

	void
	set(TYPE const &x, TYPE const &y, TYPE const &z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	void
	set(Vec3<TYPE> const &n)
	{
		this->x = n.x;
		this->y = n.y;
		this->z = n.z;
	}

	Vec3<TYPE> &
	operator=(Vec3<TYPE> const &rhs)
	{
		if (this != &rhs)
		{
			this->x = rhs.x;
			this->y = rhs.y;
			this->z = rhs.z;
		}
		return (*this);
	}

	Vec3<TYPE>
	operator+(Vec3<TYPE> const &rhs) const
	{
		return (Vec3<TYPE>(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z));
	}

	Vec3<TYPE>
	operator-(Vec3<TYPE> const &rhs) const
	{
		return (Vec3<TYPE>(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z));
	}

	Vec3<TYPE>
	operator-()
	{
		return (Vec3<TYPE>(-this->x, -this->y, -this->z));
	}

	Vec3<TYPE>
	operator*(Vec3<TYPE> const &rhs) const
	{
		return (Vec3<TYPE>(this->x * rhs.x, this->y * rhs.y, this->z * rhs.z));
	}

	Vec3<TYPE>
	operator*(TYPE const &rhs)
	{
		return (Vec3<TYPE>(this->x * rhs, this->y * rhs, this->z * rhs));
	}

	Vec3<TYPE> &
	operator+=(Vec3<TYPE> const &rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		return (*this);
	}

	Vec3<TYPE> &
	operator-=(Vec3<TYPE> const &rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		return (*this);
	}
};

template<typename TYPE> std::ostream &
operator<<(std::ostream &o, Vec3<TYPE> const &i)
{
	o	<< "x: " << i.x << ", y: " << i.y << ", z: " << i.z;
	return (o);
}

#endif
