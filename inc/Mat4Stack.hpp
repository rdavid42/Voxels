#ifndef MATSTACK_HPP
# define MATSTACK_HPP

# include <stack>
# include "Mat4.hpp"
# include "Vec3.hpp"

template<typename T>
class Mat4Stack
{
public:
	std::stack<Mat4<T>>			stack;

	Mat4Stack(void)
	{
		Mat4<T>		mat;

		mat.setIdentity();
		stack.push(mat);
	}

	~Mat4Stack(void)
	{
	}

	void			push(void)
	{
		Mat4<T>		mat;

		mat = stack.top();
		stack.push(mat);
	}

	void			pop(void)
	{
		stack.pop();
	}

	Mat4<T> &		top(void)
	{
		return (stack.top());
	}

	void			rotate(float const &angle, T x, T y, T z)
	{
		stack.top().rotate(angle, x, y, z);
	}

	void			rotate(float const &angle, Vec3<T> axis)
	{
		stack.top().rotate(angle, axis.x, axis.y, axis.z);
	}

	void			translate(T const &x, T const &y, T const &z)
	{
		stack.top().translate(x, y, z);
	}

	void			translate(Vec3<T> const &axis)
	{
		stack.top().translate(axis.x, axis.y, axis.z);
	}

	void			scale(T const &x, T const &y, T const &z)
	{
		stack.top().scale(x, y, z);
	}

	void			scale(Vec3<T> const &axis)
	{
		stack.top().scale(axis.x, axis.y, axis.z);
	}

private:
	Mat4Stack(Mat4Stack const &src);
};

#endif

