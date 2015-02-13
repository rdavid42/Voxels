
#ifndef		ENGINE_HPP
# define	ENGINE_HPP

# include <iostream>
# include <stdint.h>
# include "Octree.hpp"
# include "Noise.hpp"

class Engine
{
public:
	SDL_Window			*window;
	SDL_GLContext		context;
	uint32_t			window_width;
	uint32_t			window_height;
	Octree				*octree;
	Camera				*camera;
	GLuint				cubeList;
	Noise				*noise;

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
	void				generateTerrain(void);
	void				generateFractalTerrain(void);
	Engine				&operator=(Engine const &rhs);

private:
	Engine(Engine const &src);
};

std::ostream				&operator<<(std::ostream &o, Engine const &i);

#endif
