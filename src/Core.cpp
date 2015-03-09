
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "Core.hpp"
#include "BlockItem.hpp"
#include "Block.hpp"

static int				Polygonise(Gridcell const &grid, double const &isolevel, Triangle<float> *triangles);

Core::Core(void)
{
	return ;
}

Core::~Core(void)
{
#ifdef THREAD_POOL
	this->stopThreads();
#endif
	SDL_Quit();
	delete this->octree;
	return ;
}

int
Core::sdlError(int code)
{
	std::cerr << "(" << code << ") " << "SDL Error: " << SDL_GetError() << std::endl;
	return (code);
}

void
Core::calcFPS(void)
{
	std::ostringstream	oss;

	if ((this->fps.update = SDL_GetTicks()) - this->fps.current >= 1000)
	{
		this->fps.current = this->fps.update;
		oss << this->fps.fps << " fps";
		this->fps.title = oss.str();
		this->fps.fps = 0;
	}
	this->fps.fps++;
}
// --------------------------------------------------------------------------------
// MULTI-THREADED CHUNK GENERATION
// --------------------------------------------------------------------------------

inline static float
getDensity(Noise *n, float const &x, float const &y, float const &z)
{
	return (n->octave_noise_3d(0, x, y, z));
//		  + n->octave_noise_3d(0, x, y, z) * 0.25
//		  + n->octave_noise_3d(0, x, y, z) * 0.5);
}

inline static void
getBlockColor(Vec3<float> &r, float &t, float &density)
{
	// if (z < 0)
		r.set((density + 0.2) * 3 + t, (density + 0.2) * 2 + t, (density + 0.2) + t);
/*	else if (z >= FRAC_LIMIT - 0.2)
		r.set(0.2f - t, 0.5f - t, 0.2f - t);
	else
		r.set(81.0f / 256.0f, 55.0f / 256.0f + t, 9.0f / 256.0f);*/
}

static void
generateTriangles(float const &x, float const &y, float const &z, float const &s, int *nt, Triangle<float> *t, Noise *n)
{
	Gridcell			g;
	int					i;

	g.p[0] = Vec3<float>(x, y, z);
	g.p[1] = Vec3<float>(x + s, y, z);
	g.p[2] = Vec3<float>(x + s, y + s, z);
	g.p[3] = Vec3<float>(x, y + s, z);
	g.p[4] = Vec3<float>(x, y, z + s);
	g.p[5] = Vec3<float>(x + s, y, z + s);
	g.p[6] = Vec3<float>(x + s, y + s, z + s);
	g.p[7] = Vec3<float>(x, y + s, z + s);
	for (i = 0; i < 8; ++i)
		g.val[i] = getDensity(n, g.p[i].x, g.p[i].y, g.p[i].z) > 0 ? -1 : 1;
	*nt = Polygonise(g, 0, t);
}

#ifndef THREAD_POOL

static void
generateBlock(ThreadArgs *d, float const &x, float const &y, float const &z, int const &depth)
{
	Vec3<float>					r;
	int							i;
	float						density;
	float						nx, ny, nz;
	float						color_noise;
	Block						*b;
	int							nt;
	Triangle<float>				t[5];

	nx = d->chunk->getCube()->getX() + x;// + *d->block_size / 2;
	ny = d->chunk->getCube()->getY() + y;// + *d->block_size / 2;
	nz = d->chunk->getCube()->getZ() + z;// + *d->block_size / 2;
	density = getDensity(d->noise, nx, ny, nz);
	if (density > -0.5)
	{
		color_noise = d->noise->octave_noise_3d(0, nx, ny, nz) / 5;
		getBlockColor(r, color_noise, density);
# ifdef MARCHING_CUBES
		// b = (Block *)d->chunk->insert(nx, ny, nz, depth, BLOCK | GROUND, r, true);
		nt = 0;
		generateTriangles(nx, ny, nz, *d->block_size, &nt, t, d->noise);
		if (nt > 0)
		{
			b = static_cast<Block *>(d->chunk->insert(nx, ny, nz, depth, BLOCK | GROUND, r, true));
			if (b != NULL)
			{
				b->n = nt;
				b->t = new Triangle<float>[nt];
				for (i = 0; i < nt; ++i)
					b->t[i] = t[i];
			}
		}
# else
		d->chunk->insert(nx, ny, nz, depth, BLOCK | GROUND, r, true);
# endif
	}
}

static void *
generateChunkInThread(void *args)
{
	ThreadArgs					*d = (ThreadArgs *)args;
	float						x, y, z;
	int							depth;

	depth = BLOCK_DEPTH;
	for (z = 0.0f; z < *d->chunk_size; z += *d->block_size)
	{
		for (y = 0.0f; y < *d->chunk_size; y += *d->block_size)
		{
			for (x = 0.0f; x < *d->chunk_size; x += *d->block_size)
			{
				generateBlock(d, x, y, z, depth);
			}
		}
	}
	d->chunk->generated = true;
	delete d;
	return (NULL);
}

static void *
launchGeneration(void *args)
{
	Core *						e = (Core *)args;
	int							cz;
	int							cx, cy;
	pthread_t					init;
	ThreadArgs *				thread_args;

	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				if (e->chunks[cz][cy][cx] != NULL)
				{
					if (!e->chunks[cz][cy][cx]->generated)
					{
						thread_args = new ThreadArgs();
						thread_args->noise = e->noise;
						thread_args->chunk = e->chunks[cz][cy][cx];
						thread_args->inc = &e->noise_inc;
						thread_args->block_size = &e->block_size;
						thread_args->chunk_size = &e->chunk_size;
						thread_args->center = &e->center;
						pthread_create(&init, NULL, generateChunkInThread, thread_args);
						pthread_detach(init);
					}
				}
			}
		}
	}
	return (NULL);
}

void
Core::generation(void)
{
	pthread_t					init;

	pthread_create(&init, NULL, launchGeneration, this);
	pthread_detach(init);
}

#else

// THREAD POOL IMPLEMENTATION
// custom pool for chunk generation (no heap allocation for tasks)

void
Core::generateBlock(Chunk *c, float const &x, float const &y, float const &z, int const &depth)
{
	Vec3<float>					r;
	int							i;
	float						density;
	float						nx, ny, nz;
	float						color_noise;
	Block						*b;
	int							nt;
	Triangle<float>				t[5];

	nx = c->getCube()->getX() + x;// + *d->block_size / 2;
	ny = c->getCube()->getY() + y;// + *d->block_size / 2;
	nz = c->getCube()->getZ() + z;// + *d->block_size / 2;
	density = getDensity(this->noise, nx, ny, nz);
	if (density > -1)
	{
		color_noise = this->noise->octave_noise_3d(0, nx, ny, nz) / 5;
		getBlockColor(r, color_noise, density);
# ifdef MARCHING_CUBES
		// b = (Block *)this->chunk->insert(nx, ny, nz, depth, BLOCK | GROUND, r, true);
		nt = 0;
		generateTriangles(nx, ny, nz, this->block_size[depth], &nt, t, this->noise);
		if (nt > 0)
		{
			b = static_cast<Block *>(c->insert(nx, ny, nz, depth, BLOCK | GROUND, r, true));
			if (b != NULL)
			{
				b->n = nt;
				b->t = new Triangle<float>[nt];
				for (i = 0; i < nt; ++i)
					b->t[i] = t[i];
			}
		}
# else
		c->insert(nx, ny, nz, depth, BLOCK | GROUND, r, true);
# endif
	}
}

void
Core::processChunkGeneration(Chunk *c)
{
	float						x, y, z;
	int							depth;

	c->generated = false;
	depth = BLOCK_DEPTH;
	for (z = 0.0f; z < this->chunk_size; z += this->block_size[depth])
	{
		for (y = 0.0f; y < this->chunk_size; y += this->block_size[depth])
		{
			for (x = 0.0f; x < this->chunk_size; x += this->block_size[depth])
			{
				generateBlock(c, x, y, z, depth);
			}
		}
	}
	c->generated = true;
}

void *
Core::executeThread(int const &id)
{
	Chunk			*chunk;

	while (true)
	{
		// lock task queue and try to pick a task
		pthread_mutex_lock(&this->task_mutex[id]);
		this->is_task_locked[id] = true;

		// make thread wait when pool is empty
		while (this->pool_state != STOPPED && this->task_queue[id].empty())
			pthread_cond_wait(&this->task_cond[id], &this->task_mutex[id]);

		// stop thread when pool is destroyed
		if (this->pool_state == STOPPED)
		{
			// unlock to exit
			this->is_task_locked[id] = false;
			pthread_mutex_unlock(&this->task_mutex[id]);
			pthread_exit(0);
		}

		// pick task to process
		chunk = this->task_queue[id].front();
		this->task_queue[id].pop_front();

		// unlock task queue
		this->is_task_locked[id] = false;
		pthread_mutex_unlock(&this->task_mutex[id]);

		// process task
		processChunkGeneration(chunk);
	}
	return (0);
}

static void *
startThread(void *args)
{
	ThreadArgs *		ta = (ThreadArgs *)args;

	ta->core->executeThread(ta->i);
	delete ta;
	return (0);
}

int
Core::stopThreads(void)
{
	int				err;
	int				i;
	void			*res;

	// lock task queue
	for (i = 0; i < this->pool_size; ++i)
	{
		pthread_mutex_lock(&this->task_mutex[i]);
		this->is_task_locked[i] = true;
	}

	this->pool_state = STOPPED;

	// unlock task queue
	for (i = 0; i < this->pool_size; ++i)
	{
		this->is_task_locked[i] = false;
		pthread_mutex_unlock(&this->task_mutex[i]);
	}

	// notify threads that they need to exit
	for (i = 0; i < this->pool_size; ++i)
		pthread_cond_broadcast(&this->task_cond[i]);

	// wait for threads to exit and join them
	err = -1;
	for (i = 0; i < this->pool_size; ++i)
	{
		err = pthread_join(this->threads[i], &res);
		(void)err;
		(void)res;
		// notify threads waiting
		pthread_cond_broadcast(&this->task_cond[i]);
	}

	// destroy mutex and cond
	for (i = 0; i < this->pool_size; ++i)
	{
		pthread_mutex_destroy(&this->task_mutex[i]);
		pthread_cond_destroy(&this->task_cond[i]);
	}

	// release memory
	delete [] this->task_cond;
	delete [] this->is_task_locked;
	delete [] this->task_mutex;
	delete [] this->threads;
	delete [] this->task_queue;
	return (1);
}

uint32_t
Core::getConcurrentThreads()
{
	// just a hint
	return (std::thread::hardware_concurrency());
}

int
Core::startThreads(void)
{
	int				err;
	int				i;
	ThreadArgs		*ta;

	this->pool_size = this->getConcurrentThreads() + 1;
	if (this->pool_size <= 0)
		this->pool_size = 1;
	std::cerr << "Concurrent threads: " << this->pool_size << std::endl;
	// Thread pool heap allocation, because of variable size
	for (i = 0; i < this->pool_size; ++i)
	{
		this->task_cond = new pthread_cond_t[this->pool_size];
		this->is_task_locked = new bool[this->pool_size];
		this->task_mutex = new pthread_mutex_t[this->pool_size];
		this->threads = new pthread_t[this->pool_size];
		this->task_queue = new std::deque<Chunk *>[this->pool_size];
	}
	// mutex and cond initialization
	for (i = 0; i < this->pool_size; ++i)
	{
		pthread_mutex_init(&this->task_mutex[i], NULL);
		pthread_cond_init(&this->task_cond[i], NULL);
	}
	this->pool_state = STARTED;
	err = -1;
	for (i = 0; i < this->pool_size; ++i)
	{
		ta = new ThreadArgs();
		ta->i = i;
		ta->core = this;
		err = pthread_create(&threads[i], NULL, startThread, ta);
		if (err != 0)
		{
			std::cerr << "Failed to create Thread: " << err << std::endl;
			return (0);
		}
		else
			std::cerr << "[" << i << "] Thread created: " << std::hex << threads[i] << std::endl;
	}
	return (1);
}

void
Core::addTask(Chunk *c, int const &id)
{
	// lock task queue
	pthread_mutex_lock(&this->task_mutex[id]);
	this->is_task_locked[id] = true;

	// push task in queue
	this->task_queue[id].push_front(c);

	// clear thread task queues if they exceed TASK_QUEUE_OVERFLOW
	while (this->task_queue[id].size() > TASK_QUEUE_OVERFLOW)
		this->task_queue[id].pop_back();
	// wake up a thread to process task
	pthread_cond_signal(&this->task_cond[id]);

	// unlock task queue
	this->is_task_locked[id] = false;
	pthread_mutex_unlock(&this->task_mutex[id]);
}

void
Core::generation(void)
{
	int							cx, cy, cz;
	int							id;
	int							i;

	// get new chunks inside rendering area and add them to generation queues
	id = 0;
	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				if (this->chunks[cz][cy][cx] != NULL)
				{
					if (!this->chunks[cz][cy][cx]->generated)
						this->addTask(this->chunks[cz][cy][cx], id);
					id++;
					id %= this->pool_size;
				}
			}
		}
	}
}

#endif
// --------------------------------------------------------------------------------

void
Core::generateChunks(void)
{
	Chunk *			current = (Chunk *)this->octree->insert(camera->getPosition().x,
														camera->getPosition().y,
														camera->getPosition().z,
														CHUNK_DEPTH, CHUNK | EMPTY, Vec3<float>(1.0f, 0.0f, 1.0f), false);

	if (current != NULL)
	{
		// only try to generate if the camera moved to another chunk
		if (current != chunks[center][center][center])
		{
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
Core::insertChunks(void)
{
	int					cx, cy, cz;
	float				px, py, pz;
	Chunk *				new_chunk;

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
					new_chunk = (Chunk *)octree->insert(px, py, pz, CHUNK_DEPTH, CHUNK | EMPTY, Vec3<float>(0.7f, 0.527f, 0.0f), false);
					if (new_chunk != chunks[cz][cy][cx])
					{
						new_chunk->pos.x = cx;
						new_chunk->pos.y = cy;
						new_chunk->pos.z = cz;
						chunks[cz][cy][cx] = new_chunk;
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
// only render chunks in front of the camera
// --------------------------------------------------------------------------------
void
Core::renderChunks(void)
{
	int						cx, cy, cz;
	Chunk const *			chk; // chunk pointer
	Vec3<float> const &		cam = this->camera->getPosition(); // camera position
	Vec3<float>	const		fwr = this->camera->getForward(); // camera forward vector
	Vec3<float>				chk_cam_vec;

	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				chk = chunks[cz][cy][cx];
				if (chk != NULL && chk->generated)
				{
					if (cx == center && cy == center && cz == center)
						chk->render();
					else
					{
						// only render chunks in front of camera, using dot product
						chk_cam_vec.x = chk->getCube().getX() - cam.x;
						chk_cam_vec.y = chk->getCube().getY() - cam.y;
						chk_cam_vec.z = chk->getCube().getZ() - cam.z;
						if (fwr.dotProduct(chk_cam_vec) > CAMERA_FRONT_LIMIT)
							chk->render();
					}
				}
			}
		}
	}
}
// --------------------------------------------------------------------------------

void
Core::drawMinimap(void)
{
	int					cx, cy;
	static const int	mcs = MINIMAP_SIZE / GEN_SIZE; // chunk size on map
	static const int	mx = this->window_width - MINIMAP_SIZE + MINIMAP_PADDING; // minimap x
	static const int	my = MINIMAP_PADDING; // minimap y
	int					mcx;
	int					mcy;
	Chunk const *		chk;

	// draw map white background
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2i(mx, my);
	glVertex2i(mx + MINIMAP_SIZE - mcs - 2, my);
	glVertex2i(mx, my + MINIMAP_SIZE - mcs - 1);

	glVertex2i(mx + MINIMAP_SIZE - mcs - 2, my);
	glVertex2i(mx + MINIMAP_SIZE - mcs - 2, my + MINIMAP_SIZE - mcs - 1);
	glVertex2i(mx, my + MINIMAP_SIZE - mcs - 1);
	glEnd();
	// draw chunks edges
	glBegin(GL_LINES);
	for (cy = 0; cy < GEN_SIZE; ++cy)
	{
		for (cx = 0; cx < GEN_SIZE; ++cx)
		{
			chk = chunks[center][cy][cx];
			if (chk != NULL)
			{
				mcx = mx + cx * mcs;
				mcy = my + cy * mcs;
				if (chk->generated)
					glColor3f(0.7f, 0.5f, 0.0f);
				else
					glColor3f(1.0f, 0.0f, 0.0f);
				// up, left to right
				glVertex2i(mcx, mcy);
				glVertex2i(mcx + mcs, mcy);
				// right, up to down
				glVertex2i(mcx + mcs, mcy);
				glVertex2i(mcx + mcs, mcy + mcs);
				// down, right to left
				glVertex2i(mcx + mcs, mcy + mcs);
				glVertex2i(mcx, mcy + mcs);
				// left, down to up
				glVertex2i(mcx, mcy + mcs);
				glVertex2i(mcx, mcy);
			}
		}
	}
	// draw camera direction
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2i(mx + MINIMAP_SIZE / 2 - 1, my + MINIMAP_SIZE / 2);
	glVertex2i(mx + MINIMAP_SIZE / 2 + this->camera->getForward().x * 30, my + MINIMAP_SIZE / 2 + this->camera->getForward().y * 30);
	glEnd();
}

int
Core::getDisplayMode(void)
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

void
Core::initChunks(void)
{
	int				x, y, z;
	int				i;

	for (z = 0; z < GEN_SIZE; ++z)
		for (y = 0; y < GEN_SIZE; ++y)
			for (x = 0; x < GEN_SIZE; ++x)
				chunks[z][y][x] = NULL;
	center = (GEN_SIZE - 1) / 2;
	chunk_size = OCTREE_SIZE / powf(2, CHUNK_DEPTH);
	for (i = 1; i < MAX_BLOCK_DEPTH; ++i)
		block_size[i] = chunk_size / powf(2, i);
	// Create initial chunk
	chunks[center][center][center] = (Chunk *)octree->insert(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z,
															CHUNK_DEPTH, CHUNK | EMPTY, Vec3<float>(1.0f, 0.0f, 1.0f), false);
	this->insertChunks();
	this->generation();
}

void
Core::initSettings(void)
{
	this->hide_ui = false;
}

int
Core::init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return (sdlError(0));
	this->fps.fps = 0;
	this->fps.current = 0;
	this->fps.update = 0;
	this->octree = NULL;
	this->player = new Player;
	this->player->name = "None";
	this->window_width = 1920;
	this->window_height = 1080;
	this->highlight = NULL;
	this->particles = new ParticleEngine;
	this->initSettings();
	this->noise = new Noise(42, 256);
	this->noise->configs.emplace_back(4, 0.7, 0.2, 0.7, 0.1);
	this->noise->configs.emplace_back(FRAC_LIMIT, 10.0, 0.3, 0.2, 0.7);
	this->noise->configs.emplace_back(5, 0.4, 1, 0.2, 1);
	srandom(time(NULL));
	std::cout	<< "octaves:     " << this->noise->configs.at(0).octaves << std::endl
				<< "frequency:   " << this->noise->configs.at(0).frequency << std::endl
				<< "lacunarity:  " << this->noise->configs.at(0).lacunarity << std::endl
				<< "amplitude:   " << this->noise->configs.at(0).amplitude << std::endl
				<< "persistence: " << this->noise->configs.at(0).persistence << std::endl;
	SDL_ShowCursor(SDL_DISABLE);
	this->window = SDL_CreateWindow("Voxels",
									SDL_WINDOWPOS_UNDEFINED,
									SDL_WINDOWPOS_UNDEFINED,
									this->window_width,
									this->window_height,
									SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);//SDL_WINDOW_FULLSCREEN);
	if (this->window == NULL)
		return (sdlError(0));
	if (!(this->context = SDL_GL_CreateContext(this->window)))
		return (sdlError(0));
	SDL_SetRelativeMouseMode(SDL_TRUE);
	// glClearColor(0.527f, 0.804f, 0.917f, 1.0f);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	// glViewport(0, 0, this->window_width, this->window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, (double)this->window_width / (double)this->window_height, 0.05, OCTREE_SIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	this->camera = new Camera(Vec3<float>(0.0f, 0.0f, 0.0f));
#ifdef THREAD_POOL
	this->startThreads();
#endif
	// clock_t startTime = clock();
	this->octree = new Link(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	this->initChunks();
	glMatrixMode(GL_PROJECTION);
	return (1);
}

void
Core::renderAxes(void)
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
Core::glEnable2D(int cam_x, int cam_y)
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
Core::glDisable2D(void)
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void
Core::drawDebugInfo(void)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	this->drawText(5, 20, ("fps: " + fps.title).c_str(), GLUT_BITMAP_8_BY_13);
}

void
Core::drawText(int const x, int const y, char const *text, void *font)
{
	(void)x;
	(void)y;
	(void)text;
	int			i;
	int	const	len = strlen(text);

	glRasterPos2i(x, y);
	for (i = 0; i < len; ++i)
		glutBitmapCharacter(font, text[i]);
}

void
Core::drawUI(void)
{
	std::stringstream	str;
	int					w2 = this->window_width / 2;
	int					h2 = this->window_height / 2;
	int					i, x, y;
	int const			p = 12;

#ifdef DEBUG

	glColor3f(1.0f, 1.0f, 1.0f);
	y = 32;
	str.clear();
	str.str("");
	str << "x = ";
	str << this->camera->getPosition().x;
	drawText(5, y, str.str().c_str(), GLUT_BITMAP_8_BY_13);
	y += p;
	str.clear();
	str.str("");
	str << "y = ";
	str << this->camera->getPosition().y;
	drawText(5, y, str.str().c_str(), GLUT_BITMAP_8_BY_13);
	y += p;
	str.clear();
	str.str("");
	str << "z = ";
	str << this->camera->getPosition().z;
	drawText(5, y, str.str().c_str(), GLUT_BITMAP_8_BY_13);
	y += p;

# ifdef THREAD_POOL
	x = 130;
	y = 20;
	for (i = 0; i < this->pool_size; ++i)
	{
		str.clear();
		str.str("");
		str << "[T" << i << "] ";
		str << "tasks: ";
		str << this->task_queue[i].size();
		drawText(x, y, str.str().c_str(), GLUT_BITMAP_8_BY_13);
		y += p;
	}
# endif

#endif

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2i(w2 - 10, h2);
	glVertex2i(w2 + 10, h2);
	glVertex2i(w2, h2 - 10);
	glVertex2i(w2, h2 + 10);
	glEnd();
	if (this->player->creative)
	{
		this->displayWheel();
		this->drawText(650, 30, "Creative Mode", GLUT_BITMAP_TIMES_ROMAN_24);
	}
	this->player->inventory->drawInventory();
}

void
Core::render(void)
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
	this->particles->particleGestion();
	// this->octree->renderGround();
	//this->renderHUD();
	glMatrixMode(GL_MODELVIEW);
// --------------------------------------
// 2D
// --------------------------------------
	glEnable2D(0, 0);
	if (!this->hide_ui)
	{
		drawUI();
		drawMinimap();
#ifdef DEBUG
		this->drawDebugInfo();
#endif
	}
	glDisable2D();
// --------------------------------------
// force gl draw
// --------------------------------------
	glFlush();
}

void
Core::update(Uint32 const &elapsed_time)
{
	this->camera->animate(elapsed_time, *this);
	// std::cout << *this->camera << std::endl;
}

void
Core::removeBlock(void)
{
	Vec3<float>			inc = this->camera->getForward();
	Block *				hit; // block
	Chunk *				chunk;
	float				i;

	inc.x *= this->chunk_size;
	inc.y *= this->chunk_size;
	inc.z *= this->chunk_size;
	i = 0.0f;
	while (i < TARGET_DIST)
	{
		chunk = (Chunk *)this->octree->search(this->camera->getPosition().x + inc.x * i,
											this->camera->getPosition().y + inc.y * i,
											this->camera->getPosition().z + inc.z * i, EMPTY);
		hit = (Block *)chunk->search(this->camera->getPosition().x + inc.x * i,
									this->camera->getPosition().y + inc.y * i,
									this->camera->getPosition().z + inc.z * i, GROUND);
		if (hit != NULL && hit->getState() & GROUND)
		{
			chunk = (Chunk *)this->octree->search(this->camera->getPosition().x + inc.x * i,
												this->camera->getPosition().y + inc.y * i,
												this->camera->getPosition().z + inc.z * i, EMPTY);
			hit = (Block *)chunk->search(this->camera->getPosition().x + inc.x * i,
										this->camera->getPosition().y + inc.y * i,
										this->camera->getPosition().z + inc.z * i, GROUND);
			if (hit != NULL && hit->getState() & GROUND)
			{
				std::cerr << hit->getState();
				BlockItem			block(Vec3<float>(hit->c.x, hit->c.y, hit->c.z));
				hit->remove();
				this->particles->blockDestruction(
						Vec3<float> (this->camera->getPosition().x + inc.x * i,
										this->camera->getPosition().y + inc.y * i,
										this->camera->getPosition().z + inc.z * i),
						Vec3<float>(hit->c.x, hit->c.y, hit->c.z), hit->n);
				this->player->inventory->add(block);
				break;
			}
			i += 0.01f;
			std::cerr << hit->getState();
			BlockItem			block(Vec3<float>(hit->c.x, hit->c.y, hit->c.z));
			hit->remove();
			this->player->inventory->add(block);
			break;
		}
		i += 0.01f;
	}
}

void
Core::addInventoryBlock(void)
{
	int			xpos;
	int			ypos;
	int			win_width;
	int			win_height;
	GLfloat		pixel_color[3];
	float		blockColor[3];

	SDL_GetWindowSize(this->window, &win_width, &win_height);

	SDL_GetMouseState(&xpos, &ypos);
	glReadPixels(xpos, win_height - ypos, 1, 1, GL_RGB, GL_FLOAT, &pixel_color);
	blockColor[0] = pixel_color[0];
	blockColor[1] = pixel_color[1];
	blockColor[2] = pixel_color[2];
	BlockItem			block(Vec3<float>(blockColor[0], blockColor[1], blockColor[2]));
	this->player->inventory->add(block);
}

void
Core::onMouseButton(SDL_MouseButtonEvent const &e)
{
	if (e.type == SDL_MOUSEBUTTONDOWN)
	{
		if (!this->player->creative)
			removeBlock();
		else
			addInventoryBlock();
	}
}

void
Core::addBlock(void)
{
	Vec3<float>			inc = this->camera->getForward();
	float				chunkS;

	chunkS = OCTREE_SIZE / powf(2, CHUNK_DEPTH);
	Vec3<float>	blockPos(chunkS * inc.x, chunkS * inc.y, chunkS * inc.z);
	if (this->player->inventory->stock[this->player->inventory->selected] != NULL)
	{
		this->octree->insert(this->camera->getPosition().x + blockPos.x,
							this->camera->getPosition().y + blockPos.y,
							this->camera->getPosition().z + blockPos.z, BLOCK_DEPTH + CHUNK_DEPTH, BLOCK | GROUND,
							this->player->inventory->stock[this->player->inventory->selected]->color, false);
		this->player->inventory->deleteSelected();
	}
}

void
Core::displayWheel(void)
{
	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.0f, 1.0f);
	glVertex2i(10, 1040);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2i(140, 1040);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2i(140, 1170);
	glColor3f(1.0, 1.0f, 0.0f);
	glVertex2i(10, 1170);
	glEnd();
}

void
Core::onMouseMotion(SDL_MouseMotionEvent const &e)
{
	Vec3<float>			inc = this->camera->getForward();
	Block *				hit; // block
	float				i;

	inc.x *= this->chunk_size;
	inc.y *= this->chunk_size;
	inc.z *= this->chunk_size;
	this->highlight = NULL;
	i = 0.0f;
	while (i < TARGET_DIST)
	{
		hit = (Block *)this->octree->search(this->camera->getPosition().x + inc.x * i,
									this->camera->getPosition().y + inc.y * i,
									this->camera->getPosition().z + inc.z * i, GROUND);
		if (hit != NULL && hit->getState() & GROUND)
		{
			this->highlight = hit;
			break;
		}
		i += 0.01f;
	}
	this->camera->onMouseMotion(e);
}

void
Core::onMouseWheel(SDL_MouseWheelEvent const &e)
{
	this->camera->onMouseWheel(e);
}

void
Core::onKeyboard(SDL_KeyboardEvent const &e)
{
	this->camera->onKeyboard(e);

	if (e.type == SDL_KEYDOWN)
	{
		if (e.keysym.scancode == SDL_SCANCODE_Z)
			this->addBlock();
		if (e.keysym.scancode == SDL_SCANCODE_C)
		{
			this->player->changeMode();
			this->displayWheel();
		}
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
		if (e.keysym.scancode == SDL_SCANCODE_H)
			this->hide_ui = !this->hide_ui;
	}
}

void
Core::loop(void)
{
	SDL_Event		e;
	int32_t			quit;
	Uint32			current_time = 0;
	Uint32			elapsed_time = 0;
	Uint32			last_time = 0;
	// clock_t			startTime;

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
					if (!mouse_button && !this->player->creative)
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
		SDL_SetWindowTitle(this->window, fps.title.c_str());
		this->update(elapsed_time);
		// startTime = clock();
		this->render();
		// std::cerr << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
		SDL_GL_SwapWindow(this->window);
	}
}

Core &
Core::operator=(Core const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream &
operator<<(std::ostream &o, Core const &i)
{
	o	<< "Core: " << &i;
	return (o);
}

inline static double
dabs(double const &n)
{
	if (n < 0.0)
		return (-n);
	return (n);
}

// --------------------------------------------------------------------------------
// Linearly interpolate the position where an isosurface cuts
// an edge between two vertices, each with their own scalar value
// --------------------------------------------------------------------------------
inline static Vec3<float>
VertexInterp(double const &isolevel, Vec3<float> p1, Vec3<float> p2, double const &valp1, double const &valp2)
{
	double			mu;
	Vec3<float>		p;

	if (dabs(isolevel - valp1) < 0.00001)
		return (p1);
	if (dabs(isolevel - valp2) < 0.00001)
		return (p2);
	if (dabs(valp1 - valp2) < 0.00001)
		return (p1);
	mu = (isolevel - valp1) / (valp2 - valp1);
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);
	return (p);
}

// --------------------------------------------------------------------------------
// Given a grid cell and an isolevel, calculate the triangular
// facets required to represent the isosurface through the cell.
// Return the number of triangular facets, the array "triangles"
// will be loaded up with the vertices at most 5 triangular facets.
// 0 will be returned if the grid cell is either totally above
// of totally below the isolevel.
// --------------------------------------------------------------------------------
static int
Polygonise(Gridcell const &grid, double const &isolevel, Triangle<float> *triangles)
{
	int						i;
	int						ntriang;
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
		triangles[ntriang].p[0] = vertlist[triTable[cubeindex][i]];
		triangles[ntriang].p[1] = vertlist[triTable[cubeindex][i + 1]];
		triangles[ntriang].p[2] = vertlist[triTable[cubeindex][i + 2]];
		ntriang++;
	}

	return (ntriang);
}
