
#ifndef		ENGINE_HPP
# define	ENGINE_HPP

# include <iostream>
# include <stdint.h>
# include "Octree.hpp"
# include "Noise.hpp"

class Camera;
class Octree;

class Engine
{
public:
	SDL_Window *		window;
	SDL_GLContext		context;
	uint32_t			window_width;
	uint32_t			window_height;
	Octree *			octree;
	Camera *			camera;
	GLuint				cubeList;
	Noise *				noise;
	int					gen_dist;
	int					number_chunks; // number of chunks within distance (gen_dist)
	Octree **			chunks;

	Engine(void);
	~Engine(void);

	void				compileDisplayList(void);
	void				reshape(int const &x, int const &y);
	int					sdlError(int code);
	int					getDisplayMode(void);
	int					init(void);
	void				update(Uint32 const &elapsed_time);
	void				render(void);
	void				renderAxes(void);
	void				loop(void);
	void				generateFractalTerrain(void);
	void				initChunks(void);
	void				generateChunks(void);
	void				insertChunks(void);
	void				onMouseButton(SDL_MouseButtonEvent const &e);
	void				onMouseMotion(SDL_MouseMotionEvent const &e);
	void				onMouseWheel(SDL_MouseWheelEvent const &e);
	void				onKeyboard(SDL_KeyboardEvent const &e);
	Engine &			operator=(Engine const &rhs);

private:
	Engine(Engine const &src);
};

std::ostream &			operator<<(std::ostream &o, Engine const &i);

#endif
