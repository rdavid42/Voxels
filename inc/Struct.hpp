#ifndef STRUCT_HPP
# define STRUCT_HPP

# include <Vec3.hpp>

typedef struct
{
	Vec3<float>		p[8];
	double			val[8];
}					Gridcell;

#endif