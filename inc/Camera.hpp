#ifndef CAMERA_HPP
# define CAMERA_HPP

# include "Vec3.hpp"

class Camera
{
public:
	Camera(void);
	~Camera(void);

	Vec3<float>				pos;
	Vec3<float>				lookAt;
	Vec3<float>				forward;
	Vec3<float>				up;
	Vec3<float>				right;
	float					hangle; //horizontal camera angle
	float					vangle; //vertical camera angle

	void					set(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &forward);
	void					init(void);
	void					moveForward(void);
	void					moveBackward(void);
	void					strafeRight(void);
	void					strafeLeft(void);
	void					rotate(void);
};

#endif