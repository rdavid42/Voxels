
#ifndef		CAMERA_HPP
# define	CAMERA_HPP

# include <iostream>
# include <map>

# ifdef linux
#  include <GL/glu.h>
#  include <GL/gl.h>
#  include <SDL2/SDL.h>
# endif

# ifdef __APPLE__
#  include <openGL/glu.h>
#  include <openGL/gl.h>
#  include <SDL.h>
# endif

# ifdef __WIN32__
#  include <GL/gl.h>
#  include <windows.h>
#  include <SDL.h>
# endif

# include "Engine.hpp"
# include "Vec3.hpp"

typedef std::map<SDL_Keycode, bool>				t_keyStates;
typedef std::map<std::string, SDL_Keycode>		t_keyConfig;

class Engine;

class Camera
{
public:
	Camera(Vec3<float> const &pos);
	virtual ~Camera(void);

	void									onMouseMotion(SDL_MouseMotionEvent const &e);
	void									onMouseButton(SDL_MouseButtonEvent const &e);
	void									onKeyboard(SDL_KeyboardEvent const &e);
	void									onMouseWheel(SDL_MouseWheelEvent const &e);

	void									animate(Uint32 timeStep, Engine &e);
	void									look(void);

	Vec3<float> const						&getPosition(void) const;
	Vec3<float> const						&getForward(void) const;
	Vec3<float> const						&getTarget(void) const;

	Camera									&operator=(Camera const &rhs);

private:
	double									_speed;					// camera moving speed
	double									_sensivity;				// camera sensivity relative to mouse movements
	uint32_t								_verticalMotionDelay;
	bool									_verticalMotionActive;
	int										_verticalMotionDir;
	bool									_wheelUp;
	bool									_wheelDown;
	t_keyStates								_keyStates;				// keys current state true/false
	t_keyConfig								_keyConfig;				// keys configuration
	Vec3<float>								_position;				// camera absolute position
	Vec3<float>								_target;				// absolute position of point focused
	Vec3<float>								_forward;				// forward vector
	Vec3<float>								_left;					// left vector
	double									_theta;					// horizontal rotation angle
	double									_phi;					// vertical rotation angle

	Camera(void);
	Camera(Camera const &src);

	void									calcVectors(void);
};
std::ostream				&operator<<(std::ostream &o, Camera const &i);

#endif
