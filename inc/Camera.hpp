#ifndef CAMERA_HPP
# define CAMERA_HPP

# include "Mat4.hpp"
# include "Mat4Stack.hpp"
# include "Plane.hpp"
# include "Cube.hpp"

# define TOP_PLANE			0
# define BOTTOM_PLANE		1
# define LEFT_PLANE			2
# define RIGHT_PLANE		3
# define NEAR_PLANE			4
# define FAR_PLANE			5

enum frustrum_collision
{
	INSIDE,
	OUTSIDE
};

# define FRUSTRUM_PLANES	6

class Camera
{
public:
	Camera(void);
	~Camera(void);

	Mat4<float>				view;
	Vec3<float>				pos;
	Vec3<float>				lookAt;
	Vec3<float>				forward;
	Vec3<float>				up;
	Vec3<float>				right;
	float					boostMax;
	float					boostInc;
	float					initSpeed;
	float					speed;
	float					hangle; //horizontal camera angle
	float					vangle; //vertical camera angle

	void					init(float const &x, float const &y, float const &z, float const &fov, float const &aspect, float const &near, float const &far);
	void					set(void);
	void					setView(void);
	void					initFrustrum(void);
	void					updateFrustrum(void);
	frustrum_collision		cubeInFrustrum(Cube const &cube);
	void					moveForward(void);
	void					moveBackward(void);
	void					strafeRight(void);
	void					strafeLeft(void);
	void					rotate(void);
	void					enableBoost(void);
	void					disableBoost(void);

private:
	float					_near; // near plane distance
	float					_far;  // far plane distance
	float					_fov;  // fov y angle
	float					_aspect;
	// frustrum
	float					_tang;
	float					_nearHeight;
	float					_nearWidth;
	float					_farHeight;
	float					_farWidth;
	Plane					_planes[FRUSTRUM_PLANES];
};

#endif