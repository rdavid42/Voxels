
#include "Camera.hpp"

Camera::Camera(void)
{
}

Camera::~Camera(void)
{
}

// init -> set -> setView -> initFrustrum -> updateFrustrum

void
Camera::init(float const &x, float const &y, float const &z, float const &fov, float const &aspect, float const &near, float const &far)
{
	_fov = fov;
	_aspect = aspect;
	_near = near;
	_far = far;
	initSpeed = 0.5f;
	speed = initSpeed;
	boostMax = initSpeed * 5.0f;
	boostInc = speed / 20.0f;
	pos.set(x, y, z);
	lookAt.set(0.0f, 0.0f, 0.0f);
	forward.set(lookAt - pos);
	forward.normalize();
	set();
	initFrustrum();
}

void
Camera::set(void)
{
	Mat4<float>		translation;

	up.set(0.0f, 1.0f, 0.0f);
	right.crossProduct(forward, up);
	right.normalize();
	up.crossProduct(right, forward);
	up.normalize();
	setView();
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
	updateFrustrum();
}

void
Camera::setView(void)
{
	/*
	rx		ux		-dx		0
	ry		uy		-dy		0
	rz		uz		-dz		0
	0		0		0		1
	*/
	// first column
	view[0] = right.x;
	view[4] = right.y;
	view[8] = right.z;
	view[12] = 0.0f;
	// second column
	view[1] = up.x;
	view[5] = up.y;
	view[9] = up.z;
	view[13] = 0.0f;
	// third column
	view[2] = -forward.x;
	view[6] = -forward.y;
	view[10] = -forward.z;
	view[14] = 0.0f;
	// fourth column
	view[3] = 0.0f;
	view[7] = 0.0f;
	view[11] = 0.0f;
	view[15] = 1.0f;
}

void
Camera::initFrustrum(void)
{
	_tang = static_cast<float>(tan(_fov * (M_PI / 180.0f) * 0.5f));
	_nearHeight = _near * _tang;
	_nearWidth = _nearHeight * _aspect;
	_farHeight = _far * _tang;
	_farWidth = _farHeight * _aspect;
	updateFrustrum();
}

void
Camera::updateFrustrum(void)
{
	Vec3<float>			nearCenter, nearTopLeft, nearTopRight, nearBottomLeft, nearBottomRight;
	Vec3<float>			farCenter, farTopLeft, farTopRight, farBottomLeft, farBottomRight;
	Vec3<float>			ax, ay, az;

	// compute camera x/y/z axis
	az.set(-forward.x, -forward.y, -forward.z);
	az.normalize();
	ax.crossProduct(up, az);
	ax.normalize();
	ay.crossProduct(az, ax);
	ay.normalize();

	nearCenter = pos - az * _near;
	farCenter = pos - az * _far;

	// compute near plane corners
	nearTopLeft		= nearCenter + ay * _nearHeight - ax * _nearWidth;
	nearTopRight	= nearCenter + ay * _nearHeight + ax * _nearWidth;
	nearBottomLeft	= nearCenter - ay * _nearHeight - ax * _nearWidth;
	nearBottomRight = nearCenter - ay * _nearHeight + ax * _nearWidth;

	// compute far plane corners
	farTopLeft		= farCenter  + ay * _farHeight  - ax * _farWidth;
	farTopRight		= farCenter  + ay * _farHeight  + ax * _farWidth;
	farBottomLeft	= farCenter  - ay * _farHeight  - ax * _farWidth;
	farBottomRight	= farCenter  - ay * _farHeight  + ax * _farWidth;

	// compute frustrum planes
	_planes[TOP_PLANE].set(nearTopRight, nearTopLeft, farTopLeft);
	_planes[BOTTOM_PLANE].set(nearBottomLeft, nearBottomRight, farBottomRight);
	_planes[LEFT_PLANE].set(nearTopLeft, nearBottomLeft, farBottomLeft);
	_planes[RIGHT_PLANE].set(nearBottomRight, nearTopRight, farBottomRight);
	_planes[NEAR_PLANE].set(nearTopLeft, nearTopRight, nearBottomRight);
	_planes[FAR_PLANE].set(farTopRight, farTopLeft, farBottomLeft);
}

Vec3<float>
Camera::getVertexPosition(Vec3<float> const &position, Vec3<float> const &normal) const
{
	Vec3<float>			pos(position.x, position.y, position.z);

	if (normal.x > 0.0f)
		pos.x += CHUNK_SIZE;
	if (normal.y > 0.0f)
		pos.y += CHUNK_SIZE;
	if (normal.z > 0.0f)
		pos.z += CHUNK_SIZE;
	return (pos);
}

frustrum_collision
Camera::cubeInFrustrum(Vec3<float> const &position)
{
	frustrum_collision			result = INSIDE;

	for (int i = 0; i < FRUSTRUM_PLANES; ++i)
	{
		if (_planes[i].distance(getVertexPosition(position, _planes[i].normal)) < 0.0f)
			return (OUTSIDE);
	}
	return (result);
}

void
Camera::enableBoost(void)
{
	if (speed < boostMax)
		speed += boostInc;
}

void
Camera::disableBoost(void)
{
	if (speed > initSpeed)
		speed -= boostInc;
}

void
Camera::moveForward(void)
{
	pos += forward * speed;
	set();
}

void
Camera::moveBackward(void)
{
	pos -= forward * speed;
	set();
}

void
Camera::strafeRight(void)
{
	pos += right * speed;
	set();
}

void
Camera::strafeLeft(void)
{
	pos -= right * speed;
	set();
}

void
Camera::rotate(void)
{
	float			hr;
	float			vr;

	hr = hangle * M_PI * 0.0055; //hr = hangle * M_PI / 180;
	vr = vangle * M_PI * 0.0055; //vr = vangle * M_PI / 180;
	forward.set(cos(vr) * sin(hr),
				sin(vr),
				cos(vr) * cos(hr));
	forward.normalize();
	set();
}
