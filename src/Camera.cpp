
#include "Camera.hpp"

void
Camera::set(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &forward)
{
	Mat4<float>		translation;

	up.set(0.0f, 1.0f, 0.0f);
	right.crossProduct(forward, up);
	right.normalize();
	up.crossProduct(right, forward);
	up.normalize();
	setViewMatrix(view, forward, right, up);
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
}

void
Camera::init(void)
{
	pos.set(0.0f, 5.0f, 15.0f);
	lookAt.set(0.0f, 0.0f, 0.0f);
	forward.set(lookAt - pos);
	forward.normalize();
	std::cerr << forward << std::endl;
	forward.normalize();
	setCamera(viewMatrix, pos, forward);
}

void
Camera::moveForward(void)
{
	pos += forward;
	setCamera(viewMatrix, pos, forward);
}

void
Camera::moveBackward(void)
{
	pos -= forward;
	setCamera(viewMatrix, pos, forward);
}

void
Camera::strafeRight(void)
{
	pos += right;
	setCamera(viewMatrix, pos, forward);
}

void
Camera::strafeLeft(void)
{
	pos -= cameraRight;
	setCamera(viewMatrix, pos, forward);
}

void
Camera::rotate(void)
{
	float			hr;
	float			vr;

	hr = hangle * M_PI / 180;
	vr = vangle * M_PI / 180;
	forward.set(cos(vr) * sin(hr),
					 sin(vr),
					 cos(vr) * cos (hr));
	forward.normalize();
	setCamera(viewMatrix, pos, forward);
}
