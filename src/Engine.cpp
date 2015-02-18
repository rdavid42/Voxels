
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "Engine.hpp"
#include "BlockItem.hpp"

Engine::Engine(void)
{
	return ;
}

Engine::~Engine(void)
{
	SDL_Quit();
	delete this->octree;
	return ;
}

int
Engine::sdlError(int code)
{
	std::cerr << "(" << code << ") " << "SDL Error: " << SDL_GetError() << std::endl;
	return (code);
}

void
Engine::calcFPS(void)
{
	if ((this->fps.update = SDL_GetTicks()) - this->fps.current >= 1000)
	{
		this->fps.current = this->fps.update;
		sprintf(this->fps.title, "%d fps", this->fps.fps);
		this->fps.fps = 0;
	}
	this->fps.fps++;
}
// --------------------------------------------------------------------------------
// MULTI-THREADED CHUNK GENERATION
// POSIX threads for portability
// --------------------------------------------------------------------------------
inline static void *
generateChunkInThread(void *args)
{
	Engine::t_chunkThreadArgs	*d = (Engine::t_chunkThreadArgs *)args;
	float						x, y;
	float						n;
	Vec3<float>					r;
	float						t;
	int							i;

	if (!d->chunk->generated)
	{
		d->chunk->iterated = true;
#ifdef DEBUG
		d->chunk->c.x = 1.0f;
		d->chunk->c.y = 0.0f;
		d->chunk->c.z = 0.0f;
#endif
		for (x = -(*d->chunk_size) / 2; x < (*d->chunk_size); x += *d->inc)
		{
			for (y = -(*d->chunk_size) / 2; y < (*d->chunk_size); y += *d->inc)
			{
				n = 0.0f;
				for (i = 0; i < FRAC_LIMIT; i++)
					n += d->noise->fractal(0, d->chunk->getCube()->getX() + x, d->chunk->getCube()->getY() + y, 1.5);// + noise->fractal(0, x, y, 1.5);// * sin(y);// + this->octree->getCube()->getS() / 2;
				t = ((float)random() / (float)RAND_MAX) / 30;
				if (n >= 1.5f - t * 5)
					r = Vec3<float>(1.0f - t, 1.0f - t, 1.0f - t);
				else if (n >= 1.2f - t * 5)
					r = Vec3<float>(0.9f - t, 0.9f - t, 0.9f - t);
				else if (n >= 1.1f - t * 5)
					r = Vec3<float>(0.8f, 0.8f + t, 0.8f);
				else if (n >= 0.3f)
					r = Vec3<float>(0.1f - t, 0.4f - t, 0.1f - t);
				else if (n >= 0.2f)
					r = Vec3<float>(0.2f - t, 0.5f - t, 0.2f - t);
				else if (n >= 0.0f)
					r = Vec3<float>(81.0f / 256.0f, 55.0f / 256.0f + t, 9.0f / 256.0f);
				else if (n <= -0.7f)
					r = Vec3<float>(0.3f - t, 0.3f - t, 0.5f - t);
				else if (n <= -0.6f)
					r = Vec3<float>(0.3f - t, 0.3f - t, 0.7f - t);
				else if (n <= -0.5f)
					r = Vec3<float>(0.3f - t, 0.3f - t, 0.8f - t);
				else if (n <= -0.4f)
					r = Vec3<float>(0.96f - t, 0.894f - t, 0.647f - t);
				else if (!(random() % 1000) && n <= 0.0f && n >= -0.4f)
					r = Vec3<float>(0.1f + t, 0.1 + t, 0.1f + t);
				else if (n <= -0.1f)
					r = Vec3<float>(0.4f - t, 0.4f - t, 0.4f - t);
				else if (n <= 0.0f + t * 5)
					r = Vec3<float>(0.5f - t, 0.5f - t, 0.5f - t);
				d->chunk->insert(d->chunk->getCube()->getX() + x, d->chunk->getCube()->getY() + y, n, Octree::block_depth, BLOCK, r);
			}
		}
		d->chunk->generated = true;
#ifdef DEBUG
		d->chunk->c.x = 0.0f;
		d->chunk->c.y = 1.0f;
		d->chunk->c.z = 0.0f;
#endif
	}
	return (NULL);
}

void
Engine::generation(void)
{
	clock_t						startTime = clock();
	static float const			inc = chunk_size / powf(2.0f, 6); // should be 2^5 (32), needs a technique to generate blocks below and fill gaps
	int							cz;
	int							cx, cy;
	static int					i = 0;

	// pthread_setcanceltype(PTHREAD_CANCEL_DISABLE, NULL);
	for (cz = 0; cz < GEN_SIZE; ++cz)
		for (cy = 0; cy < GEN_SIZE; ++cy)
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				if (this->chunks[cz][cy][cx] != NULL)
				{
					thread_pool[i].args.noise = this->noise;
					thread_pool[i].args.chunk = this->chunks[cz][cy][cx];
					thread_pool[i].args.inc = &inc;
					thread_pool[i].args.chunk_size = &chunk_size;
					pthread_create(&thread_pool[i].init, NULL, generateChunkInThread, &thread_pool[i].args);
// #ifdef linux // On linux, threads are canceling themself when the thread pool is full so we join them
// 					pthread_join(thread_pool[i].init, NULL);
// #endif
// #ifdef __APPLE__ // Apparently on Mac we can detach the threads because there is some kind of thread stack
					
					// We can also just make a huge pool of threads... a bit dirty but it will do the trick for now.
					pthread_detach(thread_pool[i].init);
					(++i) &= (THREAD_POOL_SIZE - 1);
// #endif
				}
			}
	std::cerr << "(pthread)(Multi threaded) Chunks generation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
}
// --------------------------------------------------------------------------------

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
	float				px, py, pz;
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
					px = camera->getPosition().x + (cx - center) * chunk_size;
					py = camera->getPosition().y + (cy - center) * chunk_size;
					pz = camera->getPosition().z + (cz - center) * chunk_size;
					// check for terrain bounds
					if (pz <= this->noise_max && pz >= this->noise_min)
					{
						new_chunk = octree->insert(px, py, pz, octree->chunk_depth, CHUNK, Vec3<float>(0.7f, 0.5f, 0.0f));
						if (new_chunk != chunks[cz][cy][cx])
							chunks[cz][cy][cx] = new_chunk;
					}
					else
					{
						// terrain generation out of bounds, no chunk insertion here !
						chunks[cz][cy][cx] = NULL;
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
// EXPERIMENTAL
// --------------------------------------------------------------------------------
void *
cleanChunksThread(void *args)
{
	Engine *		e = (Engine *)args;
	Octree *		c;
	int				cx, cy, cz;

	while (42)
	{
		for (cz = 0; cz < GEN_SIZE; ++cz)
		{
			for (cy = 0; cy < GEN_SIZE; ++cy)
			{
				for (cx = 0; cx < GEN_SIZE; ++cx)
				{
					c = e->chunks[cz][cy][cx];
					if (c != NULL)
					{
						if (c->iterated && c->generated)
						{
							if (!c->getChild(0) && !c->getChild(1) && !c->getChild(2) && !c->getChild(3)
								&& !c->getChild(4) && !c->getChild(5) && !c->getChild(6) && !c->getChild(7))
							{
								c->remove();
								e->chunks[cz][cy][cx] = NULL;
							}
						}
					}
					usleep(500);
				}
			}
		}
	}
	return (NULL);
}
// --------------------------------------------------------------------------------

void
Engine::printNoiseMinMaxApproximation(void)
{
	float			n;
	float			max;
	float			min;
	float			x, y;
	float const		inc = 0.01;
	float			i;

	max = 0.0f;
	min = 300000.0f;
	for (x = -10; x < 10; x += inc)
		for (y = -10; y < 10; y += inc)
		{
			i = 0.0f;
			n = 0.0f;
			while (i < FRAC_LIMIT)
			{
				n += noise->fractal(0, x, y, 1.5);
				i++;
			}
			if (n > max)
				max = n;
			if (n < min)
				min = n;
		}
	std::cerr << "Fractal noise - min: " << min << ", max: " << max << std::endl;
}

void
Engine::initChunks(void)
{
	int				i; // index
	int				x, y, z;
	// pthread_t		init_cleanupThread;

	for (y = 0; y < GEN_SIZE; ++y)
		for (x = 0; x < GEN_SIZE; ++x)
			for (z = 0; z < GEN_SIZE; ++z)
				chunks[z][y][x] = NULL;
	center = (GEN_SIZE - 1) / 2;
	chunk_size = OCTREE_SIZE / powf(2, CHUNK_DEPTH);
	block_size = chunk_size / powf(2, BLOCK_DEPTH);
	// this->printNoiseMinMaxApproximation();
	this->noise_min = -FRAC_LIMIT;
	this->noise_max = FRAC_LIMIT;
	// Create initial chunk
	chunks[center][center][center] = octree->insert(camera->getPosition().x,
													camera->getPosition().y,
													camera->getPosition().z,
													octree->chunk_depth, CHUNK, Vec3<float>(1.0f, 0.0f, 1.0f));
	this->insertChunks();
	this->generation();
	// pthread_create(&init_cleanupThread, NULL, cleanChunksThread, this);
	// pthread_detach(init_cleanupThread);
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
				if (chunks[cz][cy][cx] != NULL)
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
	this->fps.fps = 0;
	this->fps.current = 0;
	this->fps.update = 0;
	this->octree = NULL;
	this->player = new Player;
	this->player->name = "[MCSTF]Korky";
	this->window_width = 1400;
	this->window_height = 1400;
	this->highlight = NULL;
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
	this->noise->configs.emplace_back(FRAC_LIMIT, 0.5, 0.2, 0.4, 0.1);
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
	gluPerspective(70, (float)(this->window_width / this->window_height), 0.1, OCTREE_SIZE);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	this->camera = new Camera(Vec3<float>(0.0f, 0.0f, 0.0f));
	// clock_t startTime = clock();
	this->octree = new Octree(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	initChunks();
	glMatrixMode(GL_PROJECTION);
	return (1);
}

void
Engine::renderAxes(void)
{
	glBegin(GL_LINES);
	// X red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(OCTREE_SIZE, 0.0f, 0.0f);
	// Y green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, OCTREE_SIZE, 0.0f);
	// Z blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, OCTREE_SIZE);
	glEnd();
}

void
Engine::glEnable2D(int cam_x, int cam_y)
{
	int			vPort[4];

	glDisable(GL_DEPTH_TEST);
	glGetIntegerv(GL_VIEWPORT, vPort);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(cam_x, vPort[2] + cam_x, vPort[3] + cam_y, cam_y, -0.001, 0.001);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void
Engine::glDisable2D(void)
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void
Engine::drawDebugInfo(void)
{
	glColor3f(1.0f, 0.0f, 0.0f);
	this->drawText(10, 20, fps.title);
}

void
Engine::drawText(int const x, int const y, char const *text)
{
	int			i;
	int	const	len = strlen(text);

	glRasterPos2i(x, y);
	for (i = 0; i < len; ++i)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
}

void
Engine::drawUI(void)
{
	std::stringstream	position;
	int			w2 = this->window_width / 2;
	int			h2 = this->window_height / 2;

	glColor3f(1.0f, 0.0f, 0.0f);
	position << "x = ";
	position << this->camera->getPosition().x;
	drawText(10, 50, position.str().c_str());
	position.clear();
	position.str("");
	position << "y = ";
	position << this->camera->getPosition().y;
	drawText(10, 80, position.str().c_str());
	position.clear();
	position.str("");
	position << "z = ";
	position << this->camera->getPosition().z;
	drawText(10, 110, position.str().c_str());

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2i(w2 - 10, h2);
	glVertex2i(w2 + 10, h2);
	glVertex2i(w2, h2 - 10);
	glVertex2i(w2, h2 + 10);
	glEnd();
	this->player->inventory->drawInventory();
}

void
Engine::render(void)
{
// --------------------------------------
// Clear screen, see glClearColor in init for clear color
// --------------------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// --------------------------------------
// 3D
// --------------------------------------
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	this->camera->look();
#ifdef DEBUG
	this->renderAxes();
#endif
	glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);
	if (this->highlight != NULL)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		this->highlight->drawCubeRidges(this->highlight->getCube()->getX(),
										this->highlight->getCube()->getY(),
										this->highlight->getCube()->getZ(),
										this->highlight->getCube()->getS());
	}
	this->renderChunks();
	// this->octree->renderGround();
	//this->renderHUD();
	glMatrixMode(GL_MODELVIEW);
// --------------------------------------
// 2D
// --------------------------------------
	glEnable2D(0, 0);
	drawUI();
#ifdef DEBUG
	this->drawDebugInfo();
#endif
	glDisable2D();
// --------------------------------------
// force gl draw
// --------------------------------------
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
	if (e.type == SDL_MOUSEBUTTONDOWN)
	{
		Vec3<float>			inc = this->camera->getForward();
		Octree *			hit; // block
		Octree *			chunk;
		float				i;
		float				j;

		inc.x *= this->chunk_size;
		inc.y *= this->chunk_size;
		inc.z *= this->chunk_size;
		i = 0.0f;
		while (i < TARGET_DIST)
		{
			chunk = this->octree->search(this->camera->getPosition().x + inc.x * i,
										this->camera->getPosition().y + inc.y * i,
										this->camera->getPosition().z + inc.z * i, CHUNK);
			hit = chunk->search(this->camera->getPosition().x + inc.x * i,
								this->camera->getPosition().y + inc.y * i,
								this->camera->getPosition().z + inc.z * i, BLOCK);
			if (hit != NULL && hit->getState() == BLOCK)
			{
				BlockItem			block(Vec3<float>(hit->c.x, hit->c.y, hit->c.z));
				hit->remove();
				this->player->inventory->add(block);
				break;
			}
			i += 0.01f;
		}
	}
//	else
//		this->camera->onMouseButton(e);
}

void
Engine::addBlock(void)
{
	Vec3<float>			inc = this->camera->getForward();
	Octree *			hit; // block
	Octree *			chunk;
	float				chunkS;

	chunkS = OCTREE_SIZE / powf(2, CHUNK_DEPTH);
	Vec3<float>	blockPos(chunkS * inc.x, chunkS * inc.y, chunkS * inc.z);
	if (this->player->inventory->stock[this->player->inventory->selected] != NULL)
	{
	//		chunk = this->octree->search(this->camera->getPosition().x + inc.x * i,
	//									this->camera->getPosition().y + inc.y * i,
	//									this->camera->getPosition().z + inc.z * i, CHUNK);
		hit = this->octree->insert(this->camera->getPosition().x + blockPos.x,
							this->camera->getPosition().y + blockPos.y,
							this->camera->getPosition().z + blockPos.z, BLOCK_DEPTH + CHUNK_DEPTH, BLOCK,
							this->player->inventory->stock[this->player->inventory->selected]->color);
		this->player->inventory->deleteSelected();
	}
}

void
Engine::onMouseMotion(SDL_MouseMotionEvent const &e)
{
	Vec3<float>			inc = this->camera->getForward();
	Octree *			hit; // block
	Octree *			chunk;
	float				i;

	inc.x *= this->chunk_size;
	inc.y *= this->chunk_size;
	inc.z *= this->chunk_size;
	this->highlight = NULL;
	i = 0.0f;
	while (i < TARGET_DIST)
	{
		chunk = this->octree->search(this->camera->getPosition().x + inc.x * i,
									this->camera->getPosition().y + inc.y * i,
									this->camera->getPosition().z + inc.z * i, CHUNK);
		hit = chunk->search(this->camera->getPosition().x + inc.x * i,
							this->camera->getPosition().y + inc.y * i,
							this->camera->getPosition().z + inc.z * i, BLOCK);
		if (hit != NULL && hit->getState() == BLOCK)
		{
			this->highlight = hit;
			break;
		}
		i += 0.01f;
	}
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

	if (e.repeat == 0 && e.type == SDL_KEYDOWN)
	{
		if (e.keysym.scancode == SDL_SCANCODE_Z)
			this->addBlock();
		if (e.keysym.scancode == SDL_SCANCODE_KP_0)
			this->player->inventory->selectItem(0);
		if (e.keysym.scancode == SDL_SCANCODE_KP_1)
			this->player->inventory->selectItem(1);
		if (e.keysym.scancode == SDL_SCANCODE_KP_2)
			this->player->inventory->selectItem(2);
		if (e.keysym.scancode == SDL_SCANCODE_KP_3)
			this->player->inventory->selectItem(3);
		if (e.keysym.scancode == SDL_SCANCODE_KP_4)
			this->player->inventory->selectItem(4);
		if (e.keysym.scancode == SDL_SCANCODE_KP_5)
			this->player->inventory->selectItem(5);
		if (e.keysym.scancode == SDL_SCANCODE_KP_6)
			this->player->inventory->selectItem(6);
		if (e.keysym.scancode == SDL_SCANCODE_KP_7)
			this->player->inventory->selectItem(7);
		if (e.keysym.scancode == SDL_SCANCODE_KP_8)
			this->player->inventory->selectItem(8);
		if (e.keysym.scancode == SDL_SCANCODE_KP_9)
			this->player->inventory->selectItem(9);
	}
}

void
Engine::loop(void)
{
	SDL_Event		e;
	int32_t			quit;
	Uint32			current_time = 0;
	Uint32			elapsed_time = 0;
	Uint32			last_time = 0;

	mouse_button = false;
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
					mouse_button = true;
					this->onMouseButton(e.button);
					break;
				case SDL_MOUSEBUTTONUP:
					mouse_button = false;
					this->onMouseButton(e.button);
				case SDL_MOUSEMOTION:
					if (!mouse_button)
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
		this->calcFPS();
		SDL_SetWindowTitle(this->window, fps.title);
		this->update(elapsed_time);
		this->render();
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

#if 0
typedef struct {
   XYZ p[3];
} TRIANGLE;

typedef struct {
   XYZ p[8];
   double val[8];
} GRIDCELL;

/*
   Given a grid cell and an isolevel, calculate the triangular
   facets required to represent the isosurface through the cell.
   Return the number of triangular facets, the array "triangles"
   will be loaded up with the vertices at most 5 triangular facets.
	0 will be returned if the grid cell is either totally above
   of totally below the isolevel.
*/
int
Polygonise(GRIDCELL grid, double isolevel, Vec3<float> *triangles)
{
	int						i, ntriang;
	int						cubeindex;
	Vec3<float>				vertlist[12];
	static int const		edgeTable[256] =
	{
		0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
		0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
		0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
		0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
		0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
		0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
		0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
		0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
		0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
		0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
		0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
		0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
		0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
		0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
		0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
		0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
		0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
		0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
		0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
		0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
		0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
		0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
		0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
		0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
		0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
		0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
	};
	static int const		triTable[256][16] =
	{
		{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
		{ 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
		{ 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
		{ 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
		{ 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
		{ 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
		{ 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
		{ 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
		{ 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
		{ 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
		{ 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
		{ 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
		{ 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
		{ 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
		{ 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
		{ 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
		{ 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
		{ 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
		{ 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
		{ 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
		{ 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
		{ 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
		{ 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
		{ 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
		{ 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
		{ 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
		{ 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
		{ 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
		{ 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
		{ 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
		{ 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
		{ 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
		{ 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
		{ 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
		{ 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
		{ 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
		{ 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
		{ 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
		{ 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
		{ 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
		{ 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
		{ 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
		{ 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
		{ 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
		{ 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
		{ 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
		{ 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
		{ 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
		{ 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
		{ 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
		{ 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
		{ 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
		{ 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
		{ 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
		{ 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
		{ 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
		{ 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
		{ 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
		{ 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
		{ 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
		{ 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
		{ 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
		{ 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
		{ 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
		{ 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
		{ 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
		{ 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
		{ 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
		{ 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
		{ 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
		{ 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
		{ 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
		{ 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
		{ 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
		{ 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
		{ 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
		{ 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
		{ 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
		{ 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
		{ 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
		{ 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
		{ 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
		{ 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
		{ 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
		{ 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
		{ 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
		{ 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
		{ 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
		{ 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
		{ 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
		{ 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
		{ 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
		{ 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
		{ 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
		{ 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
		{ 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
		{ 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
		{ 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
		{ 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
		{ 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
		{ 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
		{ 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
		{ 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
		{ 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
		{ 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
		{ 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
		{ 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
		{ 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
		{ 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
		{ 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
		{ 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
		{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
	};

	/*
	Determine the index into the edge table which
	tells us which vertices are inside of the surface
	*/
	cubeindex = 0;
	if (grid.val[0] < isolevel) cubeindex |= 1;
	if (grid.val[1] < isolevel) cubeindex |= 2;
	if (grid.val[2] < isolevel) cubeindex |= 4;
	if (grid.val[3] < isolevel) cubeindex |= 8;
	if (grid.val[4] < isolevel) cubeindex |= 16;
	if (grid.val[5] < isolevel) cubeindex |= 32;
	if (grid.val[6] < isolevel) cubeindex |= 64;
	if (grid.val[7] < isolevel) cubeindex |= 128;

	/* Cube is entirely in/out of the surface */
	if (edgeTable[cubeindex] == 0)
		return (0);

	/* Find the vertices where the surface intersects the cube */
	if (edgeTable[cubeindex] & 1) 		vertlist[0] = VertexInterp(isolevel,  grid.p[0], grid.p[1], grid.val[0], grid.val[1]);
	if (edgeTable[cubeindex] & 2) 		vertlist[1] = VertexInterp(isolevel,  grid.p[1], grid.p[2], grid.val[1], grid.val[2]);
	if (edgeTable[cubeindex] & 4) 		vertlist[2] = VertexInterp(isolevel,  grid.p[2], grid.p[3], grid.val[2], grid.val[3]);
	if (edgeTable[cubeindex] & 8) 		vertlist[3] = VertexInterp(isolevel,  grid.p[3], grid.p[0], grid.val[3], grid.val[0]);
	if (edgeTable[cubeindex] & 16) 		vertlist[4] = VertexInterp(isolevel,  grid.p[4], grid.p[5], grid.val[4], grid.val[5]);
	if (edgeTable[cubeindex] & 32) 		vertlist[5] = VertexInterp(isolevel,  grid.p[5], grid.p[6], grid.val[5], grid.val[6]);
	if (edgeTable[cubeindex] & 64) 		vertlist[6] = VertexInterp(isolevel,  grid.p[6], grid.p[7], grid.val[6], grid.val[7]);
	if (edgeTable[cubeindex] & 128) 	vertlist[7] = VertexInterp(isolevel,  grid.p[7], grid.p[4], grid.val[7], grid.val[4]);
	if (edgeTable[cubeindex] & 256) 	vertlist[8] = VertexInterp(isolevel,  grid.p[0], grid.p[4], grid.val[0], grid.val[4]);
	if (edgeTable[cubeindex] & 512) 	vertlist[9] = VertexInterp(isolevel,  grid.p[1], grid.p[5], grid.val[1], grid.val[5]);
	if (edgeTable[cubeindex] & 1024) 	vertlist[10] = VertexInterp(isolevel, grid.p[2], grid.p[6], grid.val[2], grid.val[6]);
	if (edgeTable[cubeindex] & 2048) 	vertlist[11] = VertexInterp(isolevel, grid.p[3], grid.p[7], grid.val[3], grid.val[7]);

	/* Create the triangle */
	ntriang = 0;
	for (i = 0; triTable[cubeindex][i] != -1; i += 3)
	{
		triangles[ntriang].p[0] = vertlist[triTable[cubeindex][i    ]];
		triangles[ntriang].p[1] = vertlist[triTable[cubeindex][i + 1]];
		triangles[ntriang].p[2] = vertlist[triTable[cubeindex][i + 2]];
		ntriang++;
	}

	return (ntriang);
}

/*
   Linearly interpolate the position where an isosurface cuts
   an edge between two vertices, each with their own scalar value
*/
Vec3<float>
VertexInterp(double isolevel, Vec3<float> p1, Vec3<float> p2, double valp1, double valp2)
{
	double			mu;
	Vec3<float>		p;

	if (ABS(isolevel - valp1) < 0.00001)
		return (p1);
	if (ABS(isolevel - valp2) < 0.00001)
		return (p2);
	if (ABS(valp1 - valp2) < 0.00001)
		return (p1);
	mu = (isolevel - valp1) / (valp2 - valp1);
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);
	return (p);
}
#endif
