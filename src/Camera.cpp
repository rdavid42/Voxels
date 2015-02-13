
#include "Camera.hpp"

Camera::Camera(const Vec3<float> &pos)
{
	this->_position = pos;
	this->_theta = 0.0;
	this->_phi = 0.0;
	this->calcVectors();
	this->_speed = 0.0005;
	this->_sensivity = 0.05;
	this->_verticalMotionActive = false;
	this->_wheelUp = false;
	this->_wheelDown = false;
	// keys configuration
	_keyConfig["forward"] = SDLK_w;
	_keyConfig["backward"] = SDLK_s;
	_keyConfig["strafe_left"] = SDLK_a;
	_keyConfig["strafe_right"] = SDLK_d;
	_keyConfig["boost"] = SDLK_LSHIFT;
	_keyStates[_keyConfig["forward"]] = false;
	_keyStates[_keyConfig["backward"]] = false;
	_keyStates[_keyConfig["strafe_left"]] = false;
	_keyStates[_keyConfig["strafe_right"]] = false;
	_keyStates[_keyConfig["boost"]] = false;
	return ;
}

Camera::~Camera(void)
{
	return ;
}

void
Camera::onMouseMotion(SDL_MouseMotionEvent const &e)
{
	_theta -= e.xrel * _sensivity; 
	_phi -= e.yrel * _sensivity;
	this->calcVectors();
}

void
Camera::calcVectors(void)
{
	static const Vec3<float>	up(0.0f, 0.0f, 1.0f);
	double						rtmp;

	if (_phi > 89)
		_phi = 89;
	else if (_phi < -89)
		_phi = -89;
	rtmp = cos(_phi * M_PI / 180);
	_forward.x = rtmp * cos(_theta * M_PI / 180);
	_forward.y = rtmp * sin(_theta * M_PI / 180);
	_forward.z = sin(_phi * M_PI / 180);
	_left = up.crossProduct(_forward);
	_left.normalize();
	_target = _position + _forward;
}

void
Camera::onMouseButton(SDL_MouseButtonEvent const &e)
{
	if (_wheelUp && e.type == SDL_MOUSEBUTTONDOWN)
	{
		_verticalMotionActive = true;
		_verticalMotionDelay = 250;
		_verticalMotionDir = 1;
	}
	else if (_wheelDown && e.type == SDL_MOUSEBUTTONDOWN)
	{
		_verticalMotionActive = true;
		_verticalMotionDelay = 250;
		_verticalMotionDir = -1;
	}
}

void
Camera::onMouseWheel(SDL_MouseWheelEvent const &e)
{
	if (e.y < 0)
	{
		_wheelDown = true;
		_wheelUp = false;
	}
	else
	{
		_wheelUp = true;
		_wheelDown = false;
	}
}

void
Camera::onKeyboard(SDL_KeyboardEvent const &e)
{
	for (t_keyStates::iterator it = _keyStates.begin(); it != _keyStates.end(); ++it)
	{
		if (e.keysym.sym == it->first)
		{
			it->second = (e.type == SDL_KEYDOWN);
			break ;
		}
	}
}

void
Camera::animate(Uint32 timeStep)
{
	double			realSpeed = (_keyStates[_keyConfig["boost"]]) ? 10 * _speed : _speed;

	if (_keyStates[_keyConfig["forward"]])
		_position += _forward * (realSpeed * timeStep);
	if (_keyStates[_keyConfig["backward"]])
		_position -= _forward * (realSpeed * timeStep);
	if (_keyStates[_keyConfig["strafe_left"]])
		_position += _left * (realSpeed * timeStep);
	if (_keyStates[_keyConfig["strafe_right"]])
		_position -= _left * (realSpeed * timeStep);
	if (_verticalMotionActive)
	{
		if (timeStep > _verticalMotionDelay)
			_verticalMotionActive = false;
		else
			_verticalMotionDelay -= timeStep;
		_position += Vec3<float>(0, 0, _verticalMotionDir * realSpeed * timeStep);
	}
	_target = _position + _forward;
}

void
Camera::look()
{
	gluLookAt(
		_position.x,	_position.y,	_position.z,	// eye point, camera position
		_target.x,		_target.y,		_target.z,		// reference point, camera target point
		0,				0,				1			);	// up vector constant
}

Camera
&Camera::operator=(Camera const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Camera const &i)
{
	o	<< "Camera: " << &i;
	return (o);
}
