
#ifndef		TRIANGLE_HPP
# define	TRIANGLE_HPP

# include <iostream>

template<typename TYPE>
class Triangle
{
public:
	Vec3<TYPE>				p[3];

	Triangle(void)
	{
		return ;
	}

	Triangle(Vec3<TYPE> const &p1, Vec3<TYPE> const &p2, Vec3<TYPE> const &p3)
	{
		this->p[0] = p1;
		this->p[1] = p2;
		this->p[2] = p3;
	}

	Triangle(Triangle const &src)
	{
		return ;
	}

	~Triangle(void)
	{
		return ;
	}

	Triangle &
	operator=(Triangle const &rhs)
	{
		if (this != &rhs)
		{
			this->p[0] = rhs.p[0];
			this->p[1] = rhs.p[1];
			this->p[2] = rhs.p[2];
		}
		return (*this);
	}
};

template<typename TYPE> std::ostream &
operator<<(std::ostream &o, Triangle<TYPE> const &i)
{
	o	<< "Triangle: " << &i;
	return (o);
}

#endif
