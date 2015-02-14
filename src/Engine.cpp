
#include <math.h>
#include <cstdlib>
#include <ctime>
#include "Engine.hpp"

Engine::Engine(void)
{
	return ;
}

Engine::~Engine(void)
{
	SDL_Quit();
	delete this->octree;
	delete [] chunks;
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
Engine::generateFractalTerrain(void)
{
	float				x;
	float				y;
	float				n;
	float const			i = this->octree->getCube()->getS() / powf(2.0f, 11);

	for (y = -this->octree->getCube()->getS() / 2; y < this->octree->getCube()->getS(); y += i)
	{
		for (x = -this->octree->getCube()->getS() / 2; x < this->octree->getCube()->getS(); x += i)
		{
			n = noise->fractal(0, x, y, 1.5) + noise->fractal(0, x, y, 1.5) * sin(y);// + this->octree->getCube()->getS() / 2;
			this->octree->insert(x, y, n, this->octree->ground_depth, GROUND);
		}
	}
}

void
Engine::generateChunks(void)
{
	float				x = camera->getPosition().x;
	float				y = camera->getPosition().y;
	float				z = camera->getPosition().z;
	Octree *			current = octree->insert(x, y, z, octree->chunk_depth, CHUNK);
	int					i; // index

	(void)i;
	std::cerr << "current: " << current << std::endl;
/*	if (current != NULL && current != chunks[0])
	{
		for (i = 0; i < number_chunks; ++i)
			chunks[0] = NULL;
		chunks[0] = current;
		this->insertChunks();
	}*/
}

void
Engine::initChunks(void)
{
	int			i; // index

	// add 4 lines of chunks (4 * distance) and central chunk to number_chunks
	number_chunks = (gen_dist << 2) + 1;
	// get number of size (number_chunks) using triangular sequence
	for (i = 0; i < gen_dist;)
		number_chunks += i++ << 2;

	chunks = new Octree *[number_chunks];
	for (i = 0; i < number_chunks; ++i)
		chunks[i] = NULL;

	// Create initial chunk
	chunks[0] = octree->insert(camera->getPosition().x,
								camera->getPosition().y,
								camera->getPosition().z,
								octree->chunk_depth, CHUNK);

	// std::cerr << "receives: " << chunks[0] << std::endl;
	this->insertChunks();
}

void
Engine::insertChunks(void)
{
	int			i;
	int			x;
	int			y;
	float		j[3];

	i = 1;
	j[2] = chunks[0]->getCube()->getZ();
	for (x = -gen_dist; x < 0; ++x)
	{
		j[0] = chunks[0]->getCube()->getX() + x * chunks[0]->getCube()->getS();
		chunks[i++] = octree->insert(j[0], chunks[0]->getCube()->getY(), j[2], octree->chunk_depth, CHUNK);
		for (y = 1; y < gen_dist + x + 1; ++y)
		{
			// std::cerr << "x: " << x << ", y: " << y << std::endl;
			j[1] = chunks[0]->getCube()->getY() + y * chunks[0]->getCube()->getS();
			chunks[i++] = octree->insert(j[0], j[1], j[2], octree->chunk_depth, CHUNK);
			chunks[i++] = octree->insert(j[0], -j[1], j[2], octree->chunk_depth, CHUNK);
		}
	}
	for (x = 0; x < gen_dist + 1; ++x)
	{
		j[0] = chunks[0]->getCube()->getX() + x * chunks[0]->getCube()->getS();
		if (x != 0)
			chunks[i++] = octree->insert(j[0], chunks[0]->getCube()->getY(), j[2], octree->chunk_depth, CHUNK);
		for (y = 1; y < gen_dist - x + 1; ++y)
		{
			// std::cerr << "x: " << x << ", y: " << y << std::endl;
			j[1] = chunks[0]->getCube()->getY() + y * chunks[0]->getCube()->getS();
			chunks[i++] = octree->insert(j[0], j[1], j[2], octree->chunk_depth, CHUNK);
			chunks[i++] = octree->insert(j[0], -j[1], j[2], octree->chunk_depth, CHUNK);
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
	gen_dist = GEN_DIST;
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
	srandom(time(NULL));
	this->noise->configs.emplace_back(1, 0.5, 0.2, 0.4, 0.1);
	std::cout	<< "layers:     " << this->noise->configs.at(0).layers << std::endl
				<< "frequency:  " << this->noise->configs.at(0).frequency << std::endl
				<< "lacunarity: " << this->noise->configs.at(0).lacunarity << std::endl
				<< "amplitude:  " << this->noise->configs.at(0).amplitude << std::endl
				<< "gain:       " << this->noise->configs.at(0).gain << std::endl;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	// glViewport(0, 0, this->window_width, this->window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, (float)(this->window_width / this->window_height), 0.01, 1000000);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	this->camera = new Camera(Vec3<float>(0, 0, 0));
	clock_t startTime = clock();
	this->octree = new Octree(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	this->octree->grow(3);
	this->octree = this->octree->getParent();
	this->octree->grow(4);
	this->octree = this->octree->getParent();
	// this->generateFractalTerrain();
	initChunks();
	std::cout << "Octree initialization: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
	startTime = clock();
	// this->compileDisplayList();
	std::cout << "Cube list compilation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
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
	// glCallList(this->cubeList);
	this->octree->renderGround(1.0f, 1.0f, 1.0f);
	glFlush();
}

void
Engine::update(Uint32 const &elapsed_time)
{
	this->camera->animate(elapsed_time, *this);
	// std::cout << *this->camera << std::endl;
}

void
Engine::onMouseButton(SDL_MouseButtonEvent const &e)
{
	this->camera->onMouseButton(e);
}

void
Engine::onMouseMotion(SDL_MouseMotionEvent const &e)
{
	this->camera->onMouseMotion(e);
}

void
Engine::onMouseWheel(SDL_MouseWheelEvent const &e)
{
	this->camera->onMouseWheel(e);
}

void
Engine::onKeyboard(SDL_KeyboardEvent const &e)
{
	this->camera->onKeyboard(e);
}

void
Engine::loop(void)
{
	SDL_Event		e;
	int32_t			quit;
	Uint32			current_time = 0;
	Uint32			elapsed_time = 0;
	Uint32			last_time = 0;

	quit = 0;
	while (!quit)
	{
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					this->onMouseButton(e.button);
					break;
				case SDL_MOUSEBUTTONUP:
					this->onMouseButton(e.button);
				case SDL_MOUSEMOTION:
					this->onMouseMotion(e.motion);
					break;
				case SDL_MOUSEWHEEL:
					this->onMouseWheel(e.wheel);
					break;
				case SDL_KEYUP:
					this->onKeyboard(e.key);
					break;
				case SDL_KEYDOWN:
					this->onKeyboard(e.key);
					if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
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

Engine &
Engine::operator=(Engine const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream &
operator<<(std::ostream &o, Engine const &i)
{
	o	<< "Engine: " << &i;
	return (o);
}
