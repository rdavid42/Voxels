
#ifndef		ENGINE_HPP
# define	ENGINE_HPP

# include <iostream>
# include <stdint.h>
# include <thread>
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
	Noise *				noise;
	float				chunk_size; // number of chunks within distance (gen_dist)
	float				block_size;
	int					center;
	Octree				*chunks[GEN_SIZE]
								[GEN_SIZE]
								[GEN_SIZE]; // camera chunk in the center
	std::thread			thread_pool[16];

	Engine(void);
	~Engine(void);

	int					sdlError(int code);
	int					getDisplayMode(void);
	int					init(void);
	void				update(Uint32 const &elapsed_time);
	void				render(void);
	void				renderAxes(void);
	void				loop(void);
	void				generateFractalTerrain(void);
	void				initChunks(void);
	void				insertChunks(void);
	void				renderChunks(void);
	void				generation(void);
	void				generateChunks(void);
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
