
#include "Camera.hpp"

Camera::Camera(void)
{
}

Camera::~Camera(void)
{
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
}

void
Camera::init(void)
{
	speed = 0.3;
	pos.set(0.0f, 5.0f, 15.0f);
	lookAt.set(0.0f, 0.0f, 0.0f);
	forward.set(lookAt - pos);
	forward.normalize();
	std::cerr << forward << std::endl;
	forward.normalize();
	set();
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

	hr = hangle * M_PI / 180;
	vr = vangle * M_PI / 180;
	forward.set(cos(vr) * sin(hr),
				sin(vr),
				cos(vr) * cos (hr));
	forward.normalize();
	set();
}
