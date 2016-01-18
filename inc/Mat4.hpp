#ifndef MAT4_HPP
# define MAT4_HPP

# ifndef M_PI
#  define M_PI		3.141592653589793238
# endif

# include <cmath>
# include <iostream>
# include <iomanip>

template<typename TYPE>
class Mat4
{
public:
	TYPE		val[16];

	Mat4()
	{
		int		i;

		i = 0;
		while (i < 16)
		{
			this->val[i] = 0;
			i++;
		}
	}

	~Mat4(void)
	{
	}

	void
	set(TYPE const *val)
	{
		int		i;

		i = 0;
		while (i < 16)
		{
			this->val[i] = val[i];
			i++;
		}
	}

	void
	reset(void)
	{
		int		i;

		i = 0;
		while (i < 16)
		{
			this->val[i] = 0;
			++i;
		}
	}

	void
	multiply(Mat4<TYPE> mat)
	{
		Mat4<TYPE>	res;
		int			i;
		int			j;
		int			k;

		i = 0;
		while (i < 4)
		{
			j = 0;
			while (j < 4)
			{
				res.val[j * 4 + i] = 0.0f;
				k = 0;
				while (k < 4)
				{
					res.val[j * 4 + i] += this->val[k * 4 + i] * mat.val[j * 4 + k];
					k++;
				}
				j++;
			}
			i++;
		}
		*this = res;
	}

	void
	setIdentity(void)
	{
		int			i;

		i = -1;
		while (i++, i < 16)
			this->val[i] = 0.0f;
		i = -1;
		while (i++, i < 4)
			this->val[i + i * 4] = 1.0f;
	}

	void
	setTranslation(TYPE const &x, TYPE const &y, TYPE const &z)
	{
		/*
		1		0		0		0
		0		1		0		0
		0		0		1		0
		x		y		z		1
		*/

		this->setIdentity();
		this->val[12] = x;
		this->val[13] = y;
		this->val[14] = z;
	}

	void
	translate(TYPE const &x, TYPE const &y, TYPE const &z)
	{
		Mat4<TYPE>		translate;

		translate.setTranslation(x, y, z);
		*this = *this * translate;
	}

	void
	translate(Mat4<TYPE> translate)
	{
		*this = *this * translate;
	}

	void
	setRotation(float const &angle, TYPE &x,
				TYPE &y, TYPE &z)
	{
		TYPE		s = sin(angle);
		TYPE		c = cos(angle);
		TYPE		oc = 1.0 - c;
		TYPE const	h = sqrt(x * x + y * y + z * z);

		x /= h;
		y /= h;
		z /= h;
		this->val[0] = oc * x * x + c;
		this->val[1] = oc * x * y - z * s;
		this->val[2] = oc * z * x + y * s;
		this->val[3] = 0.0;
		this->val[4] = oc * x * y + z * s;
		this->val[5] = oc * y * y + c;
		this->val[6] = oc * y * z - x * s;
		this->val[7] = 0.0;
		this->val[8] = oc * z * x - y * s;
		this->val[9] = oc * y * z + x * s;
		this->val[10] = oc * z * z + c;
		this->val[11] = 0.0;
		this->val[12] = 0.0;
		this->val[13] = 0.0;
		this->val[14] = 0.0;
		this->val[15] = 1.0;
	}

	void
	rotate(float const &angle, TYPE &x, TYPE &y, TYPE &z)
	{
		Mat4<TYPE>		rotation;

		rotation.setRotation(angle / 180.0 * M_PI, x, y, z);
		*this = *this * rotation;
	}

	void
	rotate(Mat4<TYPE> rotation)
	{
		*this = *this * rotation;
	}

	void
	setScale(TYPE const &x, TYPE const &y, TYPE const &z)
	{
		this->reset();

		this->val[0] = x;
		this->val[5] = y;
		this->val[10] = z;
		this->val[15] = 1;
	}

	void
	scale(TYPE const &x, TYPE const &y, TYPE const &z)
	{
		Mat4<TYPE>	scale;

		scale.setScale(x, y, z);
		*this = *this * scale;
	}

	void
	scale(Mat4<TYPE> scale)
	{
		*this = *this * scale;
	}

	Mat4<TYPE> &
	operator=(Mat4<TYPE> const &rhs)
	{
		int		i;

		i = 0;
		if (this != & rhs)
		{
			while (i < 16)
			{
				this->val[i] = rhs.val[i];
				++i;
			}
		}
		return (*this);
	}

	Mat4<TYPE>
	operator*(Mat4<TYPE> const & rhs) const
	{
		Mat4<TYPE>	res;
		int			i;
		int			j;
		int			k;


		i = 0;
		while (i < 4)
		{
			j = 0;
			while (j < 4)
			{
				res.val[j * 4 + i] = 0.0f;
				k = 0;
				while (k < 4)
				{
					res.val[j * 4 + i] += this->val[k * 4 + i] * rhs.val[j * 4 + k];
					k++;
				}
				j++;
			}
			i++;
		}
		return (res);
	}

	Mat4<TYPE>
	operator-(Mat4<TYPE> const &rhs) const
	{
		Mat4<TYPE> res;

		int		i;

		i = 0;
		while (i < 15)
		{
			res[i] = this->val[i] - rhs[i];
			++i;
		}
		return (res);
	}

	Mat4<TYPE> &
	operator-=(Mat4<TYPE> const &rhs)
	{
		int		i;

		i = 0;
		while (i < 15)
		{
			this->val[i] = this->val[i] - rhs[i];
			++i;
		}
		return (*this);
	}

	Mat4<TYPE>
	operator+(Mat4<TYPE> const &rhs) const
	{
		Mat4<TYPE> res;

		int		i;

		i = 0;
		while (i < 15)
		{
			res[i] = this->val[i] + rhs[i];
			++i;
		}
		return (res);
	}

	Mat4<TYPE> &
	operator+=(Mat4<TYPE> const &rhs)
	{
		int		i;

		i = 0;
		while (i < 15)
		{
			this->val[i] = this->val[i] + rhs[i];
			++i;
		}
		return (*this);
	}

	TYPE &
	operator[](int const &i)
	{
		return (val[i]);
	}
};

template<typename TYPE>
std::ostream &
operator<<(std::ostream &o, Mat4<TYPE> const &mat)
{
	int		i;
	int		j;

	i = 0;
	while (i < 4)
	{
		j = 0;
		while (j < 4)
		{
			o << " " << std::setw(10) << std::setprecision(3) << mat.val[i * 4 + j] << std::left;
			++j;
		}
		o << std::endl;
		++i;
	}
	return (o);
}


#endif
