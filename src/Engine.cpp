
#include <math.h>
#include <cstdlib>
#include <ctime>
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
		//SDL_SetWindowTitle(this->window, this->player->name.c_str());
		SDL_SetWindowTitle(this->window, this->fps.title);
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

	glGetIntegerv(GL_VIEWPORT, vPort);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(cam_x, vPort[2] + cam_x, vPort[3] + cam_y, cam_y, -1, 1);
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
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);
}

void
Engine::drawUI(void)
{

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
	Vec3<float>			blockColor;

	inc.x *= this->chunk_size;
	inc.y *= this->chunk_size;
	inc.z *= this->chunk_size;
	if (this->player->inventory->stock[0] != NULL)
	{
		blockColor = this->player->inventory->getFirstBlock();
	//		chunk = this->octree->search(this->camera->getPosition().x + inc.x * i,
	//									this->camera->getPosition().y + inc.y * i,
	//									this->camera->getPosition().z + inc.z * i, CHUNK);
		hit = this->octree->insert(this->camera->getPosition().x + inc.x,
							this->camera->getPosition().y + inc.y,
							this->camera->getPosition().z + inc.z, BLOCK_DEPTH + CHUNK_DEPTH, BLOCK, blockColor);
	}
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
	if (e.keysym.scancode == SDL_SCANCODE_Z)
		this->addBlock();
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

/*
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
	std::cerr << "(Single threaded) Chunks generation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
}
*/
/*inline static void
generateChunkInThread(Noise &noise, Octree &chunk, float const &inc, float const &chunk_size)
{
	float						x, y;
	float						n;
	Vec3<float>					r;
	float						t;

	if (!chunk.generated)
	{
		chunk.generated = true;
		for (x = -chunk_size / 2; x < chunk_size; x += inc)
		{
			for (y = -chunk_size / 2; y < chunk_size; y += inc)
			{
				n = noise.fractal(0, chunk.getCube()->getX() + x, chunk.getCube()->getY() + y, 1.5);// + noise->fractal(0, x, y, 1.5);// * sin(y);// + this->octree->getCube()->getS() / 2;
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
				chunk.insert(chunk.getCube()->getX() + x, chunk.getCube()->getY() + y, n, Octree::block_depth, GROUND, r);
			}
		}
	}
}

void
Engine::generation(void)
{
	clock_t						startTime = clock();
	static float const			inc = chunk_size / powf(2.0f, 6); // should be 2^5 (32), needs a technique to generate blocks below and fill gaps
	int							cz;
	int							cx, cy;
	static std::thread			t[GEN_SIZE * GEN_SIZE * GEN_SIZE];
	int							i;

	i = 0;
	// std::cerr << "x: " << chunks[1][1][1]->getCube()->getX() << ", s: " << s << std::endl;
	for (cz = 0; cz < GEN_SIZE; ++cz)
		for (cy = 0; cy < GEN_SIZE; ++cy)
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				t[i] = std::thread(generateChunkInThread, std::ref(*noise),
															std::ref(*chunks[cz][cy][cx]),
															std::ref(inc),
															std::ref(this->chunk_size));
				t[i++].detach();
			}
	std::cerr << "(std::thread)(Multi threaded) Chunks generation: " << double(clock() - startTime) / double(CLOCKS_PER_SEC) << " seconds." << std::endl;
}
*/

// --------------------------------------------------------------------------------
// EXPERIMENTAL - single POSIX thread generating everything in an infinite loop
// --------------------------------------------------------------------------------
/*static void *
singleThreadedGeneration(void *args)
{
	Engine						*e = (Engine *)args;
	static float const			inc = e->chunk_size / powf(2.0f, 6); // should be 2^5 (32), needs a technique to generate blocks below and fill gaps
	int							cz;
	int							cx, cy;
	float						x, y;
	float						n;
	Vec3<float>					r;
	float						t;
	int							i;

	(void)args;
	while (42)
	{
		for (cz = 0; cz < GEN_SIZE; ++cz)
		{
			for (cy = 0; cy < GEN_SIZE; ++cy)
			{
				for (cx = 0; cx < GEN_SIZE; ++cx)
				{
					if (e->chunks[cz][cy][cx] != NULL && !e->chunks[cz][cy][cx]->generated)
					{
						e->chunks[cz][cy][cx]->generated = true;
						for (x = -e->chunk_size / 2; x < e->chunk_size; x += inc)
						{
							for (y = -e->chunk_size / 2; y < e->chunk_size; y += inc)
							{
								n = 0.0f;
								for (i = 0; i < FRAC_LIMIT; i++)
									n += e->noise->fractal(0, e->chunks[cz][cy][cx]->getCube()->getX() + x, e->chunks[cz][cy][cx]->getCube()->getY() + y, 1.5);// + noise->fractal(0, x, y, 1.5);// * sin(y);// + this->octree->getCube()->getS() / 2;
								t = ((float)random() / (float)RAND_MAX) / 30;
								if (n >= 0.3f)
									r = Vec3<float>(0.1f - t, 0.4f - t, 0.1f - t);
								else if (n >= 0.2f)
									r = Vec3<float>(0.2f - t, 0.5f - t, 0.2f - t);
								else if (n >= 0.0f)
									// r = Vec3<float>(0.7f - t, 0.5f - t, 0.2f - t);
									r = Vec3<float>(81.0f / 256.0f, 55.0f / 256.0f + t, 9.0f / 256.0f);
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
								e->chunks[cz][cy][cx]->insert(e->chunks[cz][cy][cx]->getCube()->getX() + x, e->chunks[cz][cy][cx]->getCube()->getY() + y, n, Octree::block_depth, BLOCK, r);
							}
						}
					}
				}
			}
		}
	}
	return (NULL);
}*/
// --------------------------------------------------------------------------------
