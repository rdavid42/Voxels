
#include "Core.hpp"

Core::Core(void)
{
}

Core::~Core(void)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	Core		*core = static_cast<Core *>(glfwGetWindowUserPointer(window));

	(void)scancode;
	(void)mods;
	(void)core;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void
cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	Core		*core = static_cast<Core *>(glfwGetWindowUserPointer(window));

	core->camera.vangle -= ((ypos - core->lastMy) * 0.05);
	if (core->camera.vangle > 89)
		core->camera.vangle = 89;
	if (core->camera.vangle < -89)
		core->camera.vangle = -89;
	core->camera.hangle -= ((xpos - core->lastMx) * 0.05);
	core->camera.hangle = fmod(core->camera.hangle, 360);
	glfwSetCursorPos(core->window, core->windowWidth / 2, core->windowHeight / 2);
	core->lastMx = core->windowWidth / 2;
	core->lastMy = core->windowHeight / 2;
}

// *************************************************************************************************

void
Core::buildProjectionMatrix(Mat4<float> &proj, float const &fov,
							float const &near, float const &far)
{
	float const			f = 1.0f / tan(fov * (M_PI / 360.0));
	float const			ratio = (1.0f * windowWidth) / windowHeight;

	proj.setIdentity();
	proj[0] = f / ratio;
	proj[1 * 4 + 1] = f;
	proj[2 * 4 + 2] = (far + near) / (near - far);
	proj[3 * 4 + 2] = (2.0f * far * near) / (near - far);
	proj[2 * 4 + 3] = -1.0f;
	proj[3 * 4 + 3] = 0.0f;
}

void
Core::getLocations(void)
{
	// attribute variables
	positionLoc = glGetAttribLocation(this->program, "position");
	textureLoc = glGetAttribLocation(this->program, "texture");
	// uniform variables
	projLoc = glGetUniformLocation(this->program, "proj_matrix");
	viewLoc = glGetUniformLocation(this->program, "view_matrix");
	objLoc = glGetUniformLocation(this->program, "obj_matrix");
}

GLuint
Core::loadTexture(char const *filename)
{
	GLuint				texture;
	Bmp					bmp;

	if (!bmp.load(filename))
		return (printError("Failed to load bmp !", 0));
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.width, bmp.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bmp.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	checkGlError(__FILE__, __LINE__);
	return (texture);
}

void
Core::loadTextures(void)
{
	tex = new GLuint[3];
	tex[0] = loadTexture("resources/grass_bottom.bmp");
	tex[1] = loadTexture("resources/grass_side.bmp");
	tex[2] = loadTexture("resources/grass_up.bmp");
}

void
glErrorCallback(GLenum			source,
				GLenum			type,
				GLuint			id,
				GLenum			severity,
				GLsizei			length,
				const GLchar	*message,
				GLvoid			*userParam)
{
	(void)userParam;
	(void)length;
	std::cerr << "OpenGL Error:" << std::endl;
	std::cerr << "=============" << std::endl;
	std::cerr << " Object ID: " << id << std::endl;
	std::cerr << " Severity:  " << severity << std::endl;
	std::cerr << " Type:      " << type << std::endl;
	std::cerr << " Source:    " << source << std::endl;
	std::cerr << " Message:   " << message << std::endl;
	glFinish();
}

void
Core::initVoxel(void)
{
	//          y
	//		    2----3
	//		   /|   /|
	//		 6----7  |
	//		 |  0-|--1   x
	//		 |/   | /
	//		 4____5
	//		z

	static GLfloat const		voxelVertices[120] =
	{
		// vertices      | texture			C	I
		// back
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //	0	0
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, //	1	1
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //	2	2
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, //	3	3
		// left
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, //	0	4
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //	2	5
		0.0f, 0.0f, 1.0f, 1.0f, 0.0f, //	4	6
		0.0f, 1.0f, 1.0f, 1.0f, 1.0f, //	6	7
		// right
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f, //	1	8
		1.0f, 1.0f, 0.0f, 0.0f, 1.0f, //	3	9
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, //	5	10
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, //	7	11
		// floor
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //	0	12
		1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //	1	13
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //	4	14
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, //	5	15
		// ceiling
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //	2	16
		1.0f, 1.0f, 0.0f, 0.0f, 1.0f, //	3	17
		0.0f, 1.0f, 1.0f, 1.0f, 0.0f, //	6	18
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, //	7	19
		// front
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //	4	20
		1.0f, 0.0f, 1.0f, 1.0f, 0.0f, //	5	21
		0.0f, 1.0f, 1.0f, 0.0f, 1.0f, //	6	22
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f  //	7	23
	};
	static GLushort const		voxelIndices[42] =
	{
		// back
		2,  1,  3,
		2,  1,  0,
		// left
		4,  6,  7,
		4,  5,  7,
		// right
		8,  10, 11,
		8,  9,  11,
		// front
		20, 21, 23,
		20, 22, 23,
		// floor
		12, 13, 15,
		12, 14, 15,
		// ceiling
		16, 17, 18,
		17, 18, 19
	};

	glGenVertexArrays(1, &voxelVao);
	glBindVertexArray(voxelVao);
	glGenBuffers(2, &voxelVbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, voxelVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 120, voxelVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void *)0);
	glEnableVertexAttribArray(textureLoc);
	glVertexAttribPointer(textureLoc, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void *)(sizeof(GLfloat) * 3));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxelVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 42, voxelIndices, GL_STATIC_DRAW);
	// texture
	checkGlError(__FILE__, __LINE__);
}

// THREAD POOL

float
getDensity(Noise *n, float const &x, float const &y, float const &z)
{
	return (n->octave_noise_3d(0, x, y, z)
		  + n->octave_noise_3d(0, x, y, z) * 0.25
		  + n->octave_noise_3d(0, x, y, z) * 0.5);
	return (n->fractal(0, x, y, z));
}

void
Core::initNoises(void)
{
	noise = new Noise(42, 256);
	noise->configs.emplace_back(4, 0.7, 0.2, 0.7, 0.1);
	noise->configs.emplace_back(FRAC_LIMIT, 10.0, 0.3, 0.2, 0.7);
	noise->configs.emplace_back(5, 0.4, 1, 0.2, 1);
	srandom(time(NULL));
	std::cout	<< "octaves:     " << this->noise->configs.at(0).octaves << std::endl
				<< "frequency:   " << this->noise->configs.at(0).frequency << std::endl
				<< "lacunarity:  " << this->noise->configs.at(0).lacunarity << std::endl
				<< "amplitude:   " << this->noise->configs.at(0).amplitude << std::endl
				<< "persistence: " << this->noise->configs.at(0).persistence << std::endl;
}

void
Core::generateBlock(Chunk *c, float const &x, float const &y, float const &z, int const &depth)
{
	float						n;
	float						nx, ny, nz;

	nx = c->getCube()->getX() + x;
	ny = c->getCube()->getY() + y;
	nz = c->getCube()->getZ() + z;
	n = 0.0f;
	for (int i = 0; i < FRAC_LIMIT; i++)
		n += noise->fractal(0, nx, y, nz);
	// if (density > 0.9 && altitude < 0.5)
	c->insert(nx, n, nz, depth, BLOCK | GROUND);
}

void
Core::processChunkGeneration(Chunk *c)
{
	float						x, z;
	int							depth;

	c->generated = false;
	depth = BLOCK_DEPTH;
	for (z = 0.0f; z < this->chunk_size; z += this->block_size[depth])
	{
		// for (y = 0.0f; y < this->chunk_size; y += this->block_size[depth])
		// {
			for (x = 0.0f; x < this->chunk_size; x += this->block_size[depth])
			{
				c->insert(c->getCube()->getX() + x, 0.0f, c->getCube()->getZ() + z, depth, BLOCK | GROUND);
				// density = noise->fractal(0, nx, ny, nz) / 3;
				generateBlock(c, x, 1.5, z, depth);
			}
		// }
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
					px = camera.pos.x + (cx - center) * chunk_size;
					py = camera.pos.y + (cy - center) * chunk_size;
					pz = camera.pos.z + (cz - center) * chunk_size;
					new_chunk = (Chunk *)octree->insert(px, py, pz, CHUNK_DEPTH, CHUNK | EMPTY);
					new_chunk->generated = false;
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
	chunks[center][center][center] = (Chunk *)octree->insert(camera.pos.x, camera.pos.y, camera.pos.z,
															CHUNK_DEPTH, CHUNK | EMPTY);
	insertChunks();
/*
	Chunk *c = chunks[center][center][center];

	std::cerr << block_size[BLOCK_DEPTH - 1] << std::endl;
	c->insert(	c->getCube()->getX(),
				c->getCube()->getY(),
				c->getCube()->getZ(),
				BLOCK_DEPTH, BLOCK | GROUND);*/
	generation();
}

int
Core::init(void)
{
	windowWidth = 1920;
	windowHeight = 1080;
	if (!glfwInit())
		return (0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(windowWidth, windowHeight, "Voxels", NULL, NULL);
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window, mode->width / 2 - windowWidth / 2, mode->height / 2 - windowHeight / 2);
	// window = glfwCreateWindow(windowWidth, windowHeight,
									// "Voxels", glfwGetPrimaryMonitor(), NULL);
	if (!window)
	{
		glfwTerminate();
		return (0);
	}
	lastMx = 0.0;
	lastMy = 0.0;
	glfwSetWindowUserPointer(window, this);
	glfwMakeContextCurrent(window); // make the opengl context of the window current on the main thread
	glfwSwapInterval(1); // VSYNC 60 fps max
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glfwDisable(GLFW_MOUSE_CURSOR);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	buildProjectionMatrix(projMatrix, 53.13f, 0.1f, 1000.0f);
	camera.init();
	if (!initShaders(vertexShader, fragmentShader, program))
		return (0);
	getLocations();
#ifndef __APPLE__
	if (glDebugMessageControlARB != NULL)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallbackARB((GLDEBUGPROCARB)glErrorCallback, NULL);
	}
#endif
	initNoises();
	multiplier = 0.0f;
	initVoxel();
	loadTextures();
	startThreads();
	octree = new Link(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	initChunks();
	return (1);
}

void
Core::update(void)
{
	camera.rotate();
	camera.set();
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		multiplier += 0.05f;
	}
	else
	{
		multiplier -= 0.1f;
		if (multiplier < 0.0f)
			multiplier = 0.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.moveForward();
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.moveBackward();
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.strafeLeft();
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.strafeRight();
}

void
Core::render(void)
{
	float		ftime = glfwGetTime();

	(void)ftime;
	glUseProgram(program);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix.val);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.view.val);
	glBindVertexArray(voxelVao);
	glBindBuffer(GL_ARRAY_BUFFER, voxelVbo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxelVbo[1]);
	ms.push();
		octree->render(*this);
	ms.pop();
}

void
Core::loop(void)
{
	double		lastTime, currentTime;
	double		frames;

	frames = 0.0;
	lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		frames += 1.0;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		update();
		render();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (currentTime - lastTime >= 1.0)
		{
			glfwSetWindowTitle(window, std::to_string(1000.0 / frames).c_str());
			frames = 0.0;
			lastTime += 1.0;
		}
	}
}
