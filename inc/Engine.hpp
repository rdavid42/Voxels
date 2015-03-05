
#ifndef		ENGINE_HPP
# define	ENGINE_HPP

# include <iostream>
# include <stdint.h>
# include <pthread.h>
# include <unistd.h>
# include <sstream>
# include <string>
# include <list>
# include "Camera.hpp"
# include "Noise.hpp"
# include "Player.hpp"
# include "Triangle.hpp"
# include "Struct.hpp"
# include "Octree.hpp"
# include "Chunk.hpp"
# include "Block.hpp"

class Camera;
class Octree;
class Link;

class Engine
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

	// Chunk thread arguments
	typedef struct
	{
		Noise			*noise;
		Chunk			*chunk;
		float const		*inc;
		float const		*block_size;
		float const		*chunk_size;
		int				*center;
		Vec3<int>		pos;
	}					cta;

	t_timer								fps;
	SDL_Window *						window;
	SDL_GLContext						context;
	uint32_t							window_width;
	uint32_t							window_height;
	Link *								octree;
	Camera *							camera;
	Noise *								noise;
	Player *							player;
	float								chunk_size; // size of a chunk
	float								block_size; // size of a block inside a chunk
	float								noise_inc; // noise function increment, smaller than block size -> less gaps
	int									center; // central chunk's index, `chunks[center][center][center]`
	Chunk								*chunks[GEN_SIZE]
												[GEN_SIZE]
												[GEN_SIZE]; // camera chunk in the center
	float								noise_min;
	float								noise_max;
	bool								mouse_button;
	Block								*highlight;

	Engine(void);
	~Engine(void);

	void				glEnable2D(int cam_x, int cam_y);
	void				glDisable2D(void);
	void				drawDebugInfo(void);
	void				drawUI(void);
	void				calcFPS(void);
	void				drawText(int const x, int const y, char const *text);
	void				displayWheel(void);
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
	void				drawMinimap(void);
	void				generation(void);
	void				generateChunks(void);
	void				cleanChunks(void);
	void				onMouseButton(SDL_MouseButtonEvent const &e);
	void				onMouseMotion(SDL_MouseMotionEvent const &e);
	void				onMouseWheel(SDL_MouseWheelEvent const &e);
	void				onKeyboard(SDL_KeyboardEvent const &e);

	int					Polygonise(Gridcell const &grid, double const &isolevel, Triangle<float> *triangles);

	Engine &			operator=(Engine const &rhs);

private:
	Engine(Engine const &src);

	void				printNoiseMinMaxApproximation(void);
};

std::ostream &			operator<<(std::ostream &o, Engine const &i);

#endif
