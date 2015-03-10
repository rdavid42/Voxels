
#ifndef		CORE_HPP
# define	CORE_HPP

# include <iostream>
# include <stdint.h>
# include <pthread.h>
# include <thread>
# include <unistd.h>
# include <sstream>
# include <string>
# ifdef THREAD_POOL
#  include <deque>
# endif
# include "Camera.hpp"
# include "Noise.hpp"
# include "Player.hpp"
# include "Triangle.hpp"
# include "Struct.hpp"
# include "Octree.hpp"
# include "Chunk.hpp"
# include "Block.hpp"
# include "ParticleEngine.hpp"

class Camera;
class Octree;
class Link;
class Core;

# ifdef THREAD_POOL

# define STARTED	0
# define STOPPED	1

typedef struct
{
	int				i;
	Core			*core;
}					ThreadArgs;

# else

// Chunk thread arguments
typedef struct
{
	Noise *			noise;
	Chunk *			chunk;
	float const *	inc;
	float const *	block_size;
	float const *	chunk_size;
	int *			center;
}					ThreadArgs;

# endif

class Core
{
public:
	// FPS
	typedef struct		s_timer
	{
		int				fps;
		int				current;
		int				update;
		std::string		title;
	}					t_timer;

	t_timer				fps;
	SDL_Window *		window;
	SDL_GLContext		context;
	uint32_t			window_width;
	uint32_t			window_height;
	Link *				octree;
	Camera *			camera;
	Noise *				noise;
	Player *			player;
	ParticleEngine *	particles;
	float				chunk_size; // size of a chunk
	float				block_size[MAX_BLOCK_DEPTH]; // size of a block inside a chunk
	int					center; // central chunk's index, `chunks[center][center][center]`
	Chunk *				chunks[GEN_SIZE]
								[GEN_SIZE]
								[GEN_SIZE]; // camera chunk in the center
	Block				*highlight;

	// settings
	bool				hide_ui;

	//thread pool
# ifdef THREAD_POOL

	int						pool_size;
	bool					pool_state;
	pthread_cond_t *		task_cond;
	volatile bool *			is_task_locked;
	pthread_mutex_t *		task_mutex;
	pthread_t *				threads;
	std::deque<Chunk *> *	task_queue; // one different pool per thread

	uint32_t				getConcurrentThreads();
	void					processChunkGeneration(Chunk *c);
	void *					executeThread(int const &id);
	int						startThreads(void);
	int						stopThreads(void);
	void					addTask(Chunk *c, int const &id);
	void					generateBlock(Chunk *c, float const &x, float const &y, float const &z, int const &depth);

# endif

	Core(void);
	~Core(void);

	void				glEnable2D(int cam_x, int cam_y);
	void				glDisable2D(void);
	void				drawDebugInfo(void);
	void				drawUI(void);
	void				calcFPS(void);
	void				drawText(int const x, int const y, char const *text, void *font);
	void				displayWheel(void);
	int					sdlError(int code);
	int					getDisplayMode(void);
	int					init(void);
	void				initSettings(void);
	void				update(Uint32 const &elapsed_time);
	void				render(void);
	void				renderAxes(void);
	void				removeBlock(void);
	void				addInventoryBlock(void);
	void				addBlock(void);
	void				loop(void);
	void				renderHUD(void);
	void				initChunks(void);
	void				insertChunks(void);
	void				renderChunks(void);
	void				drawMinimap(void);
	void				generation(void);
	void				generateChunks(void);
	void				cleanChunks(void);
	void				onMouseButton(SDL_MouseButtonEvent const &e);
	void				onMouseMotion(SDL_MouseMotionEvent const &e);
	void				onMouseWheel(SDL_MouseWheelEvent const &e);
	void				onKeyboard(SDL_KeyboardEvent const &e);

	int					Polygonise(Gridcell const &grid, double const &isolevel, Triangle<float> *triangles);

	Core &			operator=(Core const &rhs);

private:
	Core(Core const &src);

	void				printNoiseMinMaxApproximation(void);
};

std::ostream &			operator<<(std::ostream &o, Core const &i);

#endif
