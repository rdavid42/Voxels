
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include "Engine.hpp"

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
	this->octree->renderGround();
	glEndList();
}
/*
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
			this->octree->insert(x, y, n, this->octree->block_depth, GROUND, Vec3<float>(0.0f, 0.0f, 0.0f));
		}
	}
}
*/

void
Engine::generation(void)
{
	clock_t						startTime = clock();
	float						x, y;
	float						ax, ay;
	float						n;
	Vec3<float>					r;
	float						t;
	int							cx, cy, cz;
	static float const			inc = chunk_size / powf(2.0f, 6); // should be 2^5 (32), needs a technique to generate blocks below and fill gaps

	// std::cerr << "x: " << chunks[1][1][1]->getCube()->getX() << ", s: " << s << std::endl;
	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				if (!chunks[cz][cy][cx]->generated)
				{
					chunks[cz][cy][cx]->generated = true;
					for (x = -chunk_size / 2; x < chunk_size; x += inc)
					{
						for (y = -chunk_size / 2; y < chunk_size; y += inc)
						{
							ax = chunks[cz][cy][cx]->getCube()->getX() + x;
							ay = chunks[cz][cy][cx]->getCube()->getY() + y;
							n = noise->fractal(0, ax, ay, 1.5);// + noise->fractal(0, x, y, 1.5);// * sin(y);// + this->octree->getCube()->getS() / 2;
							t = ((float)random() / (float)RAND_MAX) / 30;
							if (n >= 0.3f)
								r = Vec3<float>(0.1f - t, 0.4f - t, 0.1f - t);
							else if (n >= 0.2f)
								r = Vec3<float>(0.2f - t, 0.5f - t, 0.2f - t);
							else if (n >= 0.0f)
								r = Vec3<float>(0.7f - t, 0.5f - t, 0.2f - t);
							else if (n <= -0.7f)
								r = Vec3<float>(0.3f - t, 0.3f - t, 0.5f - t);
							else if (n <= -0.6f)
								r = Vec3<float>(0.3f - t, 0.3f - t, 0.7f - t);
							else if (n <= -0.5f)
								r = Vec3<float>(0.3f - t, 0.3f - t, 0.8f - t);
							else if (n <= -0.4f)
								r = Vec3<float>(0.96f - t, 0.894f - t, 0.647f - t);
							else if (n <= -0.1f)
								r = Vec3<float>(0.4f - t, 0.4f - t, 0.4f - t);
							else if (n <= 0.0f)
								r = Vec3<float>(0.5f - t, 0.5f - t, 0.5f - t);
							chunks[cz][cy][cx]->insert(ax, ay, n, this->octree->block_depth, GROUND, r);
						}
					}
				}
			}
		}
	}
	std::cerr << "Chunks generation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
}

void
Engine::generateChunks(void)
{
	Octree *			current = this->octree->insert(camera->getPosition().x,
														camera->getPosition().y,
														camera->getPosition().z,
														octree->chunk_depth, CHUNK, Vec3<float>(1.0f, 0.0f, 1.0f));

	if (current != NULL)
	{
		// only try to generate if the camera moved to another chunk
		if (current != chunks[center][center][center])
		{
			// std::cerr << "current: " << current << std::endl;
			// chunks[center][center][center]->remove();
			chunks[center][center][center] = current;
			this->insertChunks();
			this->generation();
		}
	}
	else
	{
		// this happens if the camera moves out of the world
		std::cerr << "Camera out ! x: " << camera->getPosition().x << ", y: " << camera->getPosition().y << ", z: " << camera->getPosition().z << std::endl;
	}
}

void
Engine::insertChunks(void)
{
	int					cx, cy, cz;
	Octree *			new_chunk;

	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				// place new chunks in the camera perimeter, ignoring the central chunk
				if (cz != center || cy != center || cx != center)
				{
					new_chunk = octree->insert(camera->getPosition().x + (cx - center) * chunk_size,
												camera->getPosition().y + (cy - center) * chunk_size,
												camera->getPosition().z + (cz - center) * chunk_size,
												octree->chunk_depth, CHUNK, Vec3<float>(1.0f, 1.0f, 0.0f));
					if (new_chunk != chunks[cz][cy][cx])
					{
						chunks[cz][cy][cx] = new_chunk;
					}
				}
			}
		}
	}
}

void
Engine::initChunks(void)
{
	int				i; // index
	int				x, y, z;

	for (y = 0; y < GEN_SIZE; ++y)
		for (x = 0; x < GEN_SIZE; ++x)
			for (z = 0; z < GEN_SIZE; ++z)
				chunks[z][y][x] = NULL;
	center = (GEN_SIZE - 1) / 2;
	chunk_size = OCTREE_SIZE / powf(2, CHUNK_DEPTH);
	block_size = chunk_size / powf(2, BLOCK_DEPTH);
	// Create initial chunk
	chunks[center][center][center] = octree->insert(camera->getPosition().x,
													camera->getPosition().y,
													camera->getPosition().z,
													octree->chunk_depth, CHUNK, Vec3<float>(1.0f, 0.0f, 1.0f));
	this->insertChunks();
	this->generation();
}

void
Engine::renderChunks(void)
{
	int				cx, cy, cz;

	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				chunks[cz][cy][cx]->renderGround();
			}
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
	srandom(time(NULL));
	this->noise->configs.emplace_back(1, 0.5, 0.2, 0.4, 0.1);
	std::cout	<< "layers:     " << this->noise->configs.at(0).layers << std::endl
				<< "frequency:  " << this->noise->configs.at(0).frequency << std::endl
				<< "lacunarity: " << this->noise->configs.at(0).lacunarity << std::endl
				<< "amplitude:  " << this->noise->configs.at(0).amplitude << std::endl
				<< "gain:       " << this->noise->configs.at(0).gain << std::endl;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	glClearColor(0.527f, 0.804f, 0.917f, 1.0f);
	// glViewport(0, 0, this->window_width, this->window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, (float)(this->window_width / this->window_height), 0.01, 1000000);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	this->camera = new Camera(Vec3<float>(0.0f, 0.0f, 0.0f));
	// clock_t startTime = clock();
	this->octree = new Octree(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	// this->generateFractalTerrain();
	initChunks();
	// std::cout << "Octree initialization: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
	// startTime = clock();
	// this->compileDisplayList();
	// std::cout << "Cube list compilation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
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
	// this->renderAxes();
	glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);
	// glCallList(this->cubeList);
	this->renderChunks();
	// this->octree->renderGround(1.0f, 1.0f, 1.0f);
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
