
#include <math.h>
#include <cstdlib>
#include <ctime>
#include "Engine.hpp"

#define OCTREE_SIZE			16.0f

Engine::Engine(void)
{
	return ;
}

Engine::~Engine(void)
{
	SDL_Quit();
	delete this->octree;
	glDeleteLists(this->cubeList, 1);
	return ;
}

int
Engine::sdlError(int code)
{
	std::cerr << "(" << code << ") " << "SDL Error: " << SDL_GetError() << std::endl;
	return (code);
}

void
Engine::reshape(int const &x, int const &y)
{
	if (x < y)
		glViewport(0, (y - x) / 2, x, x);
	if (y < x)
		glViewport((x - y) / 2, 0, y, y);
}

void
Engine::compileDisplayList(void)
{
	this->cubeList = glGenLists(1);
	if (this->cubeList == 0)
	{
		std::cerr << "List error." << std::endl;
		return ;
	}
	glNewList(this->cubeList, GL_COMPILE);
	this->octree->renderGround(1.0f, 1.0f, 1.0f);
	glEndList();
}

void
Engine::generateTerrain(void)
{
	float const			val[8] = {1.0f, -0.4f, 0.6f, 1.4f , -0.4f, -0.4f, -0.4f, -0.4f};
	float const			posx[8] = {0, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.5f, -0.5f};
	float const			posy[8] = {0, 0, 0.2f, 1.0f, -0.5f, -0.5f, 0.5f, 0.5f};
	float				tmpWeight = 0;
	float				result = 0;
	float				pond = 0;
	int					i = 0;
	float				x;
	float				y;
	float				dis;
	int					b = 0;
	int32_t				state = GROUND;

	x = -OCTREE_SIZE / 2;
	while (x <= OCTREE_SIZE)
	{
		y = -OCTREE_SIZE / 2;
		while (y <= OCTREE_SIZE)
		{
			i = 0;
			b = 0;
			pond = 0;
			tmpWeight = 0;
			while (i < 8)
			{
				dis = sqrtf((x - posx[i]) * (x - posx[i]) + (y - posy[i]) * (y - posy[i]));
				pond += 1.0 / (dis * dis);
				tmpWeight += (val[i] + 0.4f) / (dis * dis);
				if (posx[i] == x && posy[i] == y)
					b = (val[i] + 0.4f);
				i++;
			}
			result = tmpWeight / pond;
			if (b != 0)
				result = b;
			if (isnan(result))
				result = 0;
			this->octree->insert(x, y, result, Octree::max_depth, state);
			y += OCTREE_SIZE / 1000;
		}
		x += OCTREE_SIZE / 1000;
	}
}

void
Engine::generateFractalTerrain(void)
{
	float				x;
	float				y;
	float				n;
	float const			i = OCTREE_SIZE / powf(2.0f, 9);

	for (y = -OCTREE_SIZE / 2; y < OCTREE_SIZE; y += i)
	{
		for (x = -OCTREE_SIZE / 2; x < OCTREE_SIZE; x += i)
		{
			n = noise->fractal(0, x, y, 0) * 10;
			this->octree->insert(x, y, n, Octree::max_depth, GROUND);
		}
	}
}

int
Engine::getDisplayMode(void)
{
	static int32_t		display_in_use = 0; /* Only using first display */
	int32_t				i;
	int32_t				display_mode_count;
	SDL_DisplayMode		mode;

	display_mode_count = SDL_GetNumDisplayModes(display_in_use);
	if (display_mode_count < 1)
		return (0);
	i = 0;
	while (i < display_mode_count)
	{
		if (SDL_GetDisplayMode(display_in_use, i, &mode) != 0)
		{
			std::cerr << "SDL_GetDisplayMode failed: " << SDL_GetError() << std::endl;
			return (0);
		}
		if (i == 0)
		{
			this->window_width = mode.w;
			this->window_height = mode.h;
		}
		++i;
	}
	return (1);
}

int
Engine::init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return (sdlError(0));
	this->octree = NULL;
	this->window_width = 1400;
	this->window_height = 1400;
	SDL_ShowCursor(SDL_DISABLE);
	this->window = SDL_CreateWindow("Mod1",
									SDL_WINDOWPOS_UNDEFINED,
									SDL_WINDOWPOS_UNDEFINED,
									this->window_width,
									this->window_height,
									SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (this->window == NULL)
		return (sdlError(0));
	if (!(this->context = SDL_GL_CreateContext(this->window)))
		return (sdlError(0));
	this->noise = new Noise(42, 256);
	this->noise->configs.emplace_back(20, 0.3, 4.0, 0.3, 0.1);
	srandom(time(NULL));
	SDL_SetRelativeMouseMode(SDL_TRUE);
	// glViewport(0, 0, this->window_width, this->window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, (float)(this->window_width / this->window_height), 0.01, 1000000);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	clock_t startTime = clock();
	this->octree = new Octree(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	// this->generateTerrain();
	this->generateFractalTerrain();
	std::cout << "Octree initialization: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
	// startTime = clock();
	this->compileDisplayList();
	// std::cout << "Cube list compilation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
	this->camera = new Camera(Vec3<float>(-3.65569, -1.27185, 4.55836));
	this->octree->grow(0);
	this->octree = this->octree->getParent();
	return (1);
}

void
Engine::renderAxes(void)
{
	glBegin(GL_LINES);
	// X red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1000.0f, 0.0f, 0.0f);
	// Y green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1000.0f, 0.0f);
	// Z blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1000.0f);
	glEnd();
}

void
Engine::render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	this->camera->look();
	this->renderAxes();
	glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);
	glCallList(this->cubeList);
	// this->octree->renderGround(1.0f, 1.0f, 1.0f);
	glFlush();
}

void
Engine::update(Uint32 const &elapsed_time)
{
	this->camera->animate(elapsed_time);
	// std::cout << *this->camera << std::endl;
}

void
Engine::loop(void)
{
	SDL_Event		event;
	int32_t			quit;
	Uint32			current_time = 0;
	Uint32			elapsed_time = 0;
	Uint32			last_time = 0;

	quit = 0;
	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					camera->onMouseButton(event.button);
					break;
				case SDL_MOUSEBUTTONUP:
					camera->onMouseButton(event.button);
				case SDL_MOUSEMOTION:
					camera->onMouseMotion(event.motion);
					break;
				case SDL_MOUSEWHEEL:
					camera->onMouseWheel(event.wheel);
					break;
				case SDL_KEYUP:
					camera->onKeyboard(event.key);
					break;
				case SDL_KEYDOWN:
					camera->onKeyboard(event.key);
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
						quit = 1;
					break;
			}
		}
		current_time = SDL_GetTicks();
		elapsed_time = current_time - last_time;
		last_time = current_time;
		this->update(elapsed_time);
		this->render();
		SDL_Delay(1000 / 60);
		SDL_GL_SwapWindow(this->window);
	}
}

Engine
&Engine::operator=(Engine const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Engine const &i)
{
	o	<< "Engine: " << &i;
	return (o);
}
