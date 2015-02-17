
#ifndef		ENGINE_HPP
# define	ENGINE_HPP

# include <iostream>
# include <stdint.h>
# include <thread>
# include <pthread.h>
# include <unistd.h>
# include "Octree.hpp"
# include "Noise.hpp"
# include "Player.hpp"

class Camera;
class Octree;
class Chunk;

class Engine
{
public:
	// FPS calculation structure (ty Gundz)
	typedef struct		s_timer
	{
		int				fps;
		int				current;
		int				update;
		char			title[128];
	}					t_timer;

	// Chunk thread argument, should be replaced by a more generic structure in the future
	typedef struct		s_chunkThreadArgs
	{
		Noise			*noise;
		Octree			*chunk;
		float const		*inc;
		float const		*chunk_size;
	}					t_chunkThreadArgs;

	// POSIX thread arguments and initialization structure (templated structure or class planned)
	typedef struct		s_thread
	{
		pthread_t			init;
		t_chunkThreadArgs	args;
	}					t_thread;

	t_timer				fps;
	SDL_Window *		window;
	SDL_GLContext		context;
	uint32_t			window_width;
	uint32_t			window_height;
	Octree *			octree;
	Camera *			camera;
	Noise *				noise;
	Player *			player;
	float				chunk_size; // number of chunks within distance (gen_dist)
	float				block_size;
	int					center;
	Octree				*chunks[GEN_SIZE]
								[GEN_SIZE]
								[GEN_SIZE]; // camera chunk in the center
	t_thread			thread_pool[THREAD_POOL_SIZE];
	float				noise_min;
	float				noise_max;
	bool				mouse_button;
	Octree				*highlight;

	Engine(void);
	~Engine(void);

	void				glEnable2D(int cam_x, int cam_y);
	void				glDisable2D(void);
	void				drawDebugInfo(void);
	void				drawUI(void);
	void				calcFPS(void);
	void				drawText(int const x, int const y, char const *text);
	int					sdlError(int code);
	int					getDisplayMode(void);
	int					init(void);
	void				update(Uint32 const &elapsed_time);
	void				render(void);
	void				renderAxes(void);
	void				addBlock(void);
	void				loop(void);
	void				renderHUD(void);
	void				initChunks(void);
	void				insertChunks(void);
	void				renderChunks(void);
	void				generation(void);
	void				generateChunks(void);
	void				cleanChunks(void);
	void				onMouseButton(SDL_MouseButtonEvent const &e);
	void				onMouseMotion(SDL_MouseMotionEvent const &e);
	void				onMouseWheel(SDL_MouseWheelEvent const &e);
	void				onKeyboard(SDL_KeyboardEvent const &e);
	Engine &			operator=(Engine const &rhs);

private:
	Engine(Engine const &src);

	void				printNoiseMinMaxApproximation(void);
};

std::ostream &			operator<<(std::ostream &o, Engine const &i);

#endif
