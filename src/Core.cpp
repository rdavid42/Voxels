
#include "Core.hpp"

Core::Core(void)
{
}

Core::~Core(void)
{
	stopThreads();
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

void
mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	Core		*core = static_cast<Core *>(glfwGetWindowUserPointer(window));

	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		core->updateLeftClick();
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
	positionLoc = glGetAttribLocation(program, "position");
	textureLoc = glGetAttribLocation(program, "texture");
	normalLoc = glGetAttribLocation(program, "normal");
	// uniform variables
	colorLoc = glGetUniformLocation(program, "color");
	projLoc = glGetUniformLocation(program, "proj_matrix");
	viewLoc = glGetUniformLocation(program, "view_matrix");
	objLoc = glGetUniformLocation(program, "obj_matrix");
	renderVoxelRidgesLoc = glGetUniformLocation(program, "renderVoxelRidges");
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	checkGlError(__FILE__, __LINE__);
	return (texture);
}

void
Core::loadTextures(void)
{
	tex = new GLuint[1];
	tex[0] = loadTexture("resources/atlas.bmp");
	// tex[0] = loadTexture("resources/grass_bottom.bmp");
	// tex[1] = loadTexture("resources/grass_side.bmp");
	// tex[2] = loadTexture("resources/grass_up.bmp");
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
Core::createSelectionCube(void)
{

	//          y
	//		    2----3
	//		   /|   /|
	//		 6----7  |
	//		 |  0-|--1   x
	//		 |/   | /
	//		 4____5
	//		z

	selectionVerticesSize = 24;
	selectionIndicesSize = 24;
	static GLfloat const		vertices[24] =
	{
		// vertices      | texture			C	I
		0.0f, 0.0f, 0.0f, // 0
		1.0f, 0.0f, 0.0f, // 1
		0.0f, 1.0f, 0.0f, // 2
		1.0f, 1.0f, 0.0f, // 3
		0.0f, 0.0f, 1.0f, // 4
		1.0f, 0.0f, 1.0f, // 5
		0.0f, 1.0f, 1.0f, // 6
		1.0f, 1.0f, 1.0f  // 7
	};
	static GLushort const		indices[24] =
	{
		0, 1,
		0, 2,
		3, 1,
		3, 2,
		4, 5,
		4, 6,
		7, 5,
		7, 6,
		2, 6,
		3, 7,
		0, 4,
		1, 5
	};

	glGenVertexArrays(1, &selectionVao);
	glBindVertexArray(selectionVao);
	glGenBuffers(2, &selectionVbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, selectionVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * selectionVerticesSize, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, selectionVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * selectionIndicesSize, indices, GL_STATIC_DRAW);
}

inline void
addVertexToMesh(std::vector<GLfloat> &mesh,
				float const &x, float const &y, float const &z,
				float const &tx, float const &ty,
				float const &nx, float const &ny, float const &nz)  // multithread
{
	mesh.push_back(x);
	mesh.push_back(y);
	mesh.push_back(z);
	mesh.push_back(tx);
	mesh.push_back(ty);
	mesh.push_back(nx);
	mesh.push_back(ny);
	mesh.push_back(nz);
}

void
Core::generateChunkMesh(Chunk *chunk, int const &depth) // multithread
{
	float					x, y, z;
	float					cx, cy, cz;
	Block					*current;
	Octree					*tmp, *up;
	float const				bs = block_size[depth];
	float const				t[3][3][4] =
	{
		{ // DIRT
			{ 0.0f,  0.2f, 0.0f, 1.0f }, // grass/dirt
			{ 0.2f,	 0.4f, 0.0f, 1.0f }, // dirt
			{ 0.4f,  0.6f, 0.0f, 1.0f }  // grass
		},
		{ // STONE
			{ 0.6f, 0.8f, 0.0f, 1.0f },
			{ 0.6f, 0.8f, 0.0f, 1.0f },
			{ 0.6f, 0.8f, 0.0f, 1.0f }
		},
		{ // COAL
			{ 0.8f, 1.0f, 0.0f, 1.0f },
			{ 0.8f, 1.0f, 0.0f, 1.0f },
			{ 0.8f, 1.0f, 0.0f, 1.0f }
		}
	};
	int						s; // side texture index
	int						bt;
	std::vector<GLfloat>	top, back, front, left, right, bottom;
	int						a, b;

	//          y
	//		    2----3
	//		   /|   /|
	//		 6----7  |
	//		 |  0-|--1   x
	//		 |/   | /
	//		 4____5
	//		z

	cx = chunk->getCube()->getX();
	cy = chunk->getCube()->getY();
	cz = chunk->getCube()->getZ();
	chunk->mesh.clear();
	std::cerr << "x: " << cx << ", y: " << cy << ", z: " << cz << ", bs: " << bs << ", cs: " << chunk_size << std::endl;
	std::cerr << "full chunk mesh size: " << (chunk_size / bs) * (chunk_size / bs) * 6 * 8 * 6 << std::endl;
	for (z = cz; z < cz + chunk_size; z += bs)
	{
		a = 0;
		for (y = cy; y < cy + chunk_size; y += bs)
		{
			for (x = cx; x < cx + chunk_size; x += bs)
			{
				current = static_cast<Block *>(chunk->search(x, y, z, BLOCK, true));
				if (current)
				{
					b = 0;
					bt = current->type - 1;
					up = chunk->search(x, y + bs, z, BLOCK, true); // top
					if (!up)
					{
						addVertexToMesh(top, x,		 y + bs, z,		 t[bt][2][0], t[bt][2][2], 0.0f, 1.0f, 0.0f); // 2
						addVertexToMesh(top, x,		 y + bs, z + bs, t[bt][2][1], t[bt][2][2], 0.0f, 1.0f, 0.0f); // 6
						addVertexToMesh(top, x + bs, y + bs, z + bs, t[bt][2][1], t[bt][2][3], 0.0f, 1.0f, 0.0f); // 7
						addVertexToMesh(top, x + bs, y + bs, z + bs, t[bt][2][1], t[bt][2][3], 0.0f, 1.0f, 0.0f); // 7
						addVertexToMesh(top, x + bs, y + bs, z,		 t[bt][2][0], t[bt][2][3], 0.0f, 1.0f, 0.0f); // 3
						addVertexToMesh(top, x,		 y + bs, z,		 t[bt][2][0], t[bt][2][2], 0.0f, 1.0f, 0.0f); // 2
						b++;
					}
					s = 0;
					if (up)
						s = 1;
					tmp = chunk->search(x, y, z - bs, BLOCK, true); // back
					if (!tmp)
					{
						addVertexToMesh(back, x,		 y,		 z, t[bt][s][0], t[bt][s][2], 0.0f, 0.0f, -1.0f); // 0
						addVertexToMesh(back, x,		 y + bs, z, t[bt][s][0], t[bt][s][3], 0.0f, 0.0f, -1.0f); // 2
						addVertexToMesh(back, x + bs, y + bs, z, t[bt][s][1], t[bt][s][3], 0.0f, 0.0f, -1.0f); // 3
						addVertexToMesh(back, x + bs, y + bs, z, t[bt][s][1], t[bt][s][3], 0.0f, 0.0f, -1.0f); // 3
						addVertexToMesh(back, x + bs, y,		 z, t[bt][s][1], t[bt][s][2], 0.0f, 0.0f, -1.0f); // 1
						addVertexToMesh(back, x,		 y,		 z, t[bt][s][0], t[bt][s][2], 0.0f, 0.0f, -1.0f); // 0
						b++;
					}
					tmp = chunk->search(x, y, z + bs, BLOCK, true); // front
					if (!tmp)
					{
						addVertexToMesh(front, x,		 y,		 z + bs, t[bt][s][0], t[bt][s][2], 1.0f, 0.0f, 0.0f); // 4
						addVertexToMesh(front, x + bs, y,		 z + bs, t[bt][s][1], t[bt][s][2], 1.0f, 0.0f, 0.0f); // 5
						addVertexToMesh(front, x + bs, y + bs, z + bs, t[bt][s][1], t[bt][s][3], 1.0f, 0.0f, 0.0f); // 7
						addVertexToMesh(front, x + bs, y + bs, z + bs, t[bt][s][1], t[bt][s][3], 1.0f, 0.0f, 0.0f); // 7
						addVertexToMesh(front, x,		 y + bs, z + bs, t[bt][s][0], t[bt][s][3], 1.0f, 0.0f, 0.0f); // 6
						addVertexToMesh(front, x,		 y,		 z + bs, t[bt][s][0], t[bt][s][2], 1.0f, 0.0f, 0.0f); // 4
						b++;
					}
					tmp = chunk->search(x - bs, y, z, BLOCK, true); // left
					if (!tmp)
					{
						addVertexToMesh(left, x, y,		z,		t[bt][s][0], t[bt][s][2], -1.0f, 0.0f, 0.0f); // 0
						addVertexToMesh(left, x, y,		z + bs, t[bt][s][1], t[bt][s][2], -1.0f, 0.0f, 0.0f); // 4
						addVertexToMesh(left, x, y + bs, z + bs, t[bt][s][1], t[bt][s][3], -1.0f, 0.0f, 0.0f); // 6
						addVertexToMesh(left, x, y + bs, z + bs, t[bt][s][1], t[bt][s][3], -1.0f, 0.0f, 0.0f); // 6
						addVertexToMesh(left, x, y + bs, z,		t[bt][s][0], t[bt][s][3], -1.0f, 0.0f, 0.0f); // 2
						addVertexToMesh(left, x, y,		z,		t[bt][s][0], t[bt][s][2], -1.0f, 0.0f, 0.0f); // 0
						b++;
					}
					tmp = chunk->search(x + bs, y, z, BLOCK, true); // right
					if (!tmp)
					{
						addVertexToMesh(right, x + bs, y,		 z,		 t[bt][s][0], t[bt][s][2], 1.0f, 0.0f, 0.0f); // 1
						addVertexToMesh(right, x + bs, y + bs, z,		 t[bt][s][0], t[bt][s][3], 1.0f, 0.0f, 0.0f); // 3
						addVertexToMesh(right, x + bs, y + bs, z + bs, t[bt][s][1], t[bt][s][3], 1.0f, 0.0f, 0.0f); // 7
						addVertexToMesh(right, x + bs, y + bs, z + bs, t[bt][s][1], t[bt][s][3], 1.0f, 0.0f, 0.0f); // 7
						addVertexToMesh(right, x + bs, y,		 z + bs, t[bt][s][1], t[bt][s][2], 1.0f, 0.0f, 0.0f); // 5
						addVertexToMesh(right, x + bs, y,		 z,		 t[bt][s][0], t[bt][s][2], 1.0f, 0.0f, 0.0f); // 1
						b++;
					}
					tmp = chunk->search(x, y - bs, z, BLOCK, true); // bottom
					if (!tmp)
					{
						addVertexToMesh(bottom, x,		 y, z,		t[bt][1][0], t[bt][1][3], 0.0f, -1.0f, 0.0f); // 0
						addVertexToMesh(bottom, x + bs, y, z,		t[bt][1][1], t[bt][1][3], 0.0f, -1.0f, 0.0f); // 1
						addVertexToMesh(bottom, x + bs, y, z + bs, t[bt][1][1], t[bt][1][2], 0.0f, -1.0f, 0.0f); // 5
						addVertexToMesh(bottom, x + bs, y, z + bs, t[bt][1][1], t[bt][1][2], 0.0f, -1.0f, 0.0f); // 5
						addVertexToMesh(bottom, x,		 y, z + bs, t[bt][1][0], t[bt][1][2], 0.0f, -1.0f, 0.0f); // 4
						addVertexToMesh(bottom, x,		 y, z,		t[bt][1][0], t[bt][1][3], 0.0f, -1.0f, 0.0f); // 0
						b++;
					}
					if (b > 0)
						a++;
				}
			}
			if (a == 0)
				std::cerr << "z: " << z << ", y: " << y << ", x: " << x << std::endl;
		}
	}
	std::cerr << a << std::endl;
	chunk->mesh.reserve(top.size() + back.size() + front.size() + left.size() + right.size() + bottom.size());
	chunk->mesh.insert(chunk->mesh.end(), top.begin(), top.end());
	chunk->mesh.insert(chunk->mesh.end(), back.begin(), back.end());
	chunk->mesh.insert(chunk->mesh.end(), front.begin(), front.end());
	chunk->mesh.insert(chunk->mesh.end(), left.begin(), left.end());
	chunk->mesh.insert(chunk->mesh.end(), right.begin(), right.end());
	chunk->mesh.insert(chunk->mesh.end(), bottom.begin(), bottom.end());
	std::cerr << "top size: " << top.size() << std::endl;
	std::cerr << "back size: " << back.size() << std::endl;
	std::cerr << "front size: " << front.size() << std::endl;
	std::cerr << "left size: " << left.size() << std::endl;
	std::cerr << "right size: " << right.size() << std::endl;
	std::cerr << "bottom size: " << bottom.size() << std::endl;
	chunk->meshSize = chunk->mesh.size() / 8;
	std::cerr << "mesh size: " << chunk->mesh.size() << std::endl;
/*	for (size_t i = 0; i < chunk->mesh.size(); i += (8 * 6))
	{
		for (size_t j = 0; j < 6; j++)
		{
			for (size_t k = 0; k < 8; k++)
			{
				std::cerr << chunk->mesh[i + j + k];
				if (k == 7)
					std::cerr << std::endl;
				else
					std::cerr << ", ";
			}
		}
	}*/
	// std::cerr << "vertices: " << chunk->meshSize << std::endl;
	// std::cerr << "triangles: " << chunk->meshSize / 3 << std::endl;
}

void
Core::simplifyChunkMesh(Chunk *chunk)
{
	(void)chunk;
}

void
Core::initNoises(void) // multithread
{
	noise = new Noise(42, 256);
	// octaves range     : 1.0 - 6.0
	// frequency range   : 0.0 - 1.0
	// lacunarity range  : ?
	// amplitude range   : > 0.0
	// persistence range : 0.0 - 10
	noise->configs.emplace_back(4, 0.01, 0.5, 0.1, 0.1); // bruit 3d test					//	0
	noise->configs.emplace_back(6, 0.008, 1.0, 0.9, 1.0); // bruit 3d équilibré				//	1
	noise->configs.emplace_back(2, 0.008, 10.0, 0.9, 1.0); // bruit 3d monde des reves		//	2
	noise->configs.emplace_back(3, 0.1, 0.1, 0.1, 0.2); // Des montagnes, mais pas trop		//	3
	noise->configs.emplace_back(6, 0.1, 0.0, 0.1, 10.0); // La vallée Danna					//	4
	noise->configs.emplace_back(1, 0.2, 0.0, 0.1, 4.0); // Les montagnes.					//	5
	noise->configs.emplace_back(5, 0.4, 1, 0.2, 1);											//	6
	srandom(time(NULL));
	std::cerr	<< "octaves:     " << this->noise->configs.at(0).octaves << std::endl
				<< "frequency:   " << this->noise->configs.at(0).frequency << std::endl
				<< "lacunarity:  " << this->noise->configs.at(0).lacunarity << std::endl
				<< "amplitude:   " << this->noise->configs.at(0).amplitude << std::endl
				<< "persistence: " << this->noise->configs.at(0).persistence << std::endl;
}

void
Core::generateBlock3d(Chunk *c, float const &x, float const &y, float const &z, int const &depth, int const &ycap) // multithread
{
	float						n;
	float						nstone;
	// float						ncoal;
	float						nx, nz, ny;
	int							i;

	nx = c->getCube()->getX() + x;
	ny = c->getCube()->getY() + y;
	nz = c->getCube()->getZ() + z;


	n = 0.0f;
	nstone = noise->fractal(5, nx, ny, nz);
	// ncoal = nstone;
	for (i = 0; i < 3; i++)
		n += noise->octave_noise_3d(i, nx, ny, nz);
	n /= (i + 1);
	if (ny > 0)
	{
		n /= (ny / ycap);
		if (n > 0.90)
		{
			if (n < 0.95 && nstone < 0.6)
			c->insert(nx, ny, nz, depth, BLOCK, DIRT); // dirt
			else
			{
				if ((nstone > 0.75 && nstone < 0.76) || (nstone > 0.65 && nstone < 0.66))
					c->insert(nx, ny, nz, depth, BLOCK, COAL);
				else
					c->insert(nx, ny, nz, depth, BLOCK, STONE); //stone
			}
		}
	}
}

void
Core::generateBlock(Chunk *c, float const &x, float const &y, float const &z, int const &depth) // multithread
{
	float                        altitude;
	float                        nx, nz;

	nx = c->getCube()->getX() + x;
	nz = c->getCube()->getZ() + z;
	altitude = 0.0f;
	for (int i = 0; i < 10.0f; i++)
		altitude += noise->fractal(2, nx, y, nz);
	for (; altitude > -25.0f; altitude -= this->block_size[depth])
		c->insert(nx, altitude, nz, depth, BLOCK, DIRT);
}

void
Core::processChunkGeneration(Chunk *chunk) // multithread
{
	float						x, z, y;
	int							depth;

	if (chunk->generated)
		return ;
	chunk->generated = false;
	depth = BLOCK_DEPTH;
	for (z = 0.0f; z < this->chunk_size; z += this->block_size[depth])
	{
		// for (y = 0.0f; y < this->chunk_size; y += this->block_size[depth])
		// {
			for (x = 0.0f; x < this->chunk_size; x += this->block_size[depth])
			{
				// chunk->insert(chunk->getCube()->getX() + x, 0.0f, chunk->getCube()->getZ() + z, depth, BLOCK | GROUND);
				// density = noise->fractal(0, nx, ny, nz) / 3;
				for (y = 0.0f; y < this->chunk_size; y += this->block_size[depth])
				{
					generateBlock3d(chunk, x, y, z, depth, 50);
					//generateBlock(chunk, x, 1.5, z, depth);
				}
			}
		// }
	}
	generateChunkMesh(chunk, depth);
	chunk->generated = true;
}

// THREAD POOL

void *
Core::executeThread(int const &id) // multithread
{
	Chunk			*chunk;

	while (true)
	{
		// lock task queue and try to pick a task
		pthread_mutex_lock(&this->task_mutex[id]);
		this->is_task_locked[id] = true;

		// make the thread wait when the pool is empty
		while (this->pool_state != STOPPED && this->task_queue[id].empty())
			pthread_cond_wait(&this->task_cond[id], &this->task_mutex[id]);

		// stop the thread when the pool is destroyed
		if (this->pool_state == STOPPED)
		{
			// unlock to exit
			this->is_task_locked[id] = false;
			pthread_mutex_unlock(&this->task_mutex[id]);
			pthread_exit(0);
		}

		// pick task to process
		chunk = this->task_queue[id].front();
		if (chunk != 0)
		{
			this->task_queue[id].pop_front();

			// unlock task queue
			this->is_task_locked[id] = false;
			pthread_mutex_unlock(&this->task_mutex[id]);

			// process task
			processChunkGeneration(chunk);
		}
	}
	return (0);
}

static void *
startThread(void *args) // multithread
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
			std::cerr << "[" << i << "] Thread created: " << threads[i] << std::endl;
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
Core::generateChunkGLMesh(Chunk *chunk)
{
	glGenVertexArrays(1, &chunk->vao);
	glBindVertexArray(chunk->vao);
	glGenBuffers(1, &chunk->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * chunk->mesh.size(), &chunk->mesh[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)0);
	glEnableVertexAttribArray(textureLoc);
	glVertexAttribPointer(textureLoc, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(normalLoc);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)(sizeof(GLfloat) * 5));
	chunk->mesh.clear();
	// destroy mesh vector
	std::vector<GLfloat>().swap(chunk->mesh);
}

void
Core::generation(void)
{
	int							cx, cy, cz;
	int							id;
	Chunk						*chunk;

	// get new chunks inside rendering area and add them to generation queues
	id = 0;
	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				if (chunks[cz][cy][cx] != NULL)
				{
					chunk = chunks[cz][cy][cx];
					if (!chunk->generated && !chunk->generating)
					{
						chunk->generating = true;
						addTask(chunk, id);
					}
					if (chunk->generated && !chunk->renderDone)
					{
						// wait for the chunk generation and allocate the opengl mesh
						// opengl functions cannot be called from a thread so we do it on the main thread (current OpenGL context)
						if (chunk->meshSize > 0)
							generateChunkGLMesh(chunk);
						chunk->renderDone = true;
					}
					id++;
					id %= pool_size;
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
				if (chunks[cz][cy][cx] == NULL)
				{
					// place new chunks in the camera perimeter, ignoring the central chunk
					if (cz != center || cy != center || cx != center)
					{
						px = camera.pos.x + (cx - center) * chunk_size;
						py = camera.pos.y + (cy - center) * chunk_size;
						pz = camera.pos.z + (cz - center) * chunk_size;
						new_chunk = (Chunk *)octree->insert(px, py, pz, CHUNK_DEPTH, CHUNK, NONE);
						if (new_chunk != NULL)
						{
							new_chunk->generated = false;
							new_chunk->generating = false;
							new_chunk->renderDone = false;
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
	}
}

Block *
Core::getClosestBlock(void)
{
	Vec3<float>			pos;
	int					i;
	int const			precision = 10;
	int const			dist = 10 * precision; // blocks max distance
	Block				*block;

	pos = camera.pos;
	for (i = 0; i < dist; ++i)
	{
		block = reinterpret_cast<Block *>(octree->search(pos.x, pos.y, pos.z, BLOCK, false));
		if (block)
			return (block);
		pos += camera.forward * (block_size[BLOCK_DEPTH] / precision);
	}
	return (NULL);
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
	chunks[center][center][center] = static_cast<Chunk *>(octree->insert(camera.pos.x, camera.pos.y, camera.pos.z,
																		CHUNK_DEPTH, CHUNK, NONE));
	chunks[center][center][center]->generated = false;
	chunks[center][center][center]->generating = false;
	chunks[center][center][center]->renderDone = false;
	insertChunks();
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glfwDisable(GLFW_MOUSE_CURSOR);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	buildProjectionMatrix(projMatrix, 53.13f, 0.1f, 1000.0f);
	camera.init();
	if (!initShaders(vertexShader, fragmentShader, program))
		return (0);
	getLocations();
/*#ifndef __APPLE__ // Mac osx doesnt support opengl 4.3+
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_FALSE);
	glDebugMessageCallback((GLDEBUGPROC)glErrorCallback, NULL);
#endif*/
	initNoises();
	loadTextures();
	createSelectionCube();
	startThreads();
	octree = new Link(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	initChunks();
	closestBlock = 0;
	return (1);
}

void
Core::updateLeftClick(void)
{
	Chunk			*chunk;

	if (closestBlock != NULL)
	{
		chunk = closestBlock->getChunk();
		closestBlock->destroy();
		glBindVertexArray(chunk->vao);
		glDeleteBuffers(1, &chunk->vbo);
		generateChunkMesh(chunk, BLOCK_DEPTH);
		generateChunkGLMesh(chunk);
	}
}

void
Core::updateChunks(void)
{
	Chunk								*central;
	int									x, y, z;
	Vec3<int>							dir;
	Chunk								*newChunks[GEN_SIZE][GEN_SIZE][GEN_SIZE];
	std::list<Chunk *>					delChunks;
	std::list<Chunk *>::iterator		it;

	for (z = 0; z < GEN_SIZE; ++z)
		for (y = 0; y < GEN_SIZE; ++y)
			for (x = 0; x < GEN_SIZE; ++x)
				newChunks[z][y][x] = NULL;
	central = static_cast<Chunk *>(octree->search(camera.pos.x, camera.pos.y, camera.pos.z, CHUNK, false));
	// std::cerr << central << std::endl;
	if (central != chunks[center][center][center])
	{
		dir.set(0, 0, 0);
		// get the vector between the current central chunk and the new one
		for (z = 0; z < GEN_SIZE; ++z)
		{
			for (y = 0; y < GEN_SIZE; ++y)
			{
				for (x = 0; x < GEN_SIZE; ++x)
				{
					if (central == chunks[z][y][x])
					{
						dir.set(x - center, y - center, z - center);
						goto nested_break;
					}
				}
			}
		}
		nested_break:
		if (dir.x == 0 && dir.y == 0 && dir.z == 0)
			return ; // outside of the generation area
		// get the new chunk disposition, depending on the new central chunk
		for (z = 0; z < GEN_SIZE; ++z)
		{
			for (y = 0; y < GEN_SIZE; ++y)
			{
				for (x = 0; x < GEN_SIZE; ++x)
				{
					if (x - dir.x < 0 || x - dir.x >= GEN_SIZE
					||	y - dir.y < 0 || y - dir.y >= GEN_SIZE
					||	z - dir.z < 0 || z - dir.z >= GEN_SIZE)
						delChunks.push_front(chunks[z][y][x]);
					else
						newChunks[z - dir.z][y - dir.y][x - dir.x] = chunks[z][y][x];
				}
			}
		}
		newChunks[center][center][center] = central;
		// copy new chunk disposition
		for (z = 0; z < GEN_SIZE; ++z)
			for (y = 0; y < GEN_SIZE; ++y)
				for (x = 0; x < GEN_SIZE; ++x)
					chunks[z][y][x] = newChunks[z][y][x];
		// delete the chunks out of the grid
		for (it = delChunks.begin(); it != delChunks.end(); ++it)
		{
			if (*it && (*it)->renderDone)
			{
				std::cerr << "deleting: " << *it << std::endl;
				glDeleteBuffers(1, &(*it)->vbo);
				glDeleteVertexArrays(1, &(*it)->vao);
				delete *it;
			}
		}
		insertChunks();
	}
}

void
Core::update(void)
{
/*	for (int z = 0; z < GEN_SIZE; ++z)
		for (int y = 0; y < GEN_SIZE; ++y)
			for (int x = 0; x < GEN_SIZE; ++x)
				std::cerr << chunks[z][y][x]->getState() << std::endl;*/
	camera.rotate();
	camera.set();
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.moveForward();
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.moveBackward();
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.strafeLeft();
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.strafeRight();
	closestBlock = getClosestBlock();
	generation();
	// updateChunks();
}

void
Core::render(void)
{
	float		ftime = glfwGetTime();
	int			x, y, z;

	(void)ftime;
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.view.val);
	ms.push();
		// render meshes
		glUniform1f(renderVoxelRidgesLoc, 0.0f);
		glBindTexture(GL_TEXTURE_2D, tex[0]);
		for (z = 0; z < GEN_SIZE; ++z)
			for (y = 0; y < GEN_SIZE; ++y)
				for (x = 0; x < GEN_SIZE; ++x)
				{
					if (chunks[z][y][x])
						chunks[z][y][x]->render(*this);
				}
		// render chunks ridges
		glBindVertexArray(selectionVao);
		glUniform1f(renderVoxelRidgesLoc, 1.0f);
		glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
/*		for (z = 0; z < GEN_SIZE; ++z)
			for (y = 0; y < GEN_SIZE; ++y)
				for (x = 0; x < GEN_SIZE; ++x)
				{
					if (chunks[z][y][x])
						chunks[z][y][x]->renderRidges(*this);
				}*/
		if (closestBlock != NULL)
			closestBlock->renderRidges(*this);
	ms.pop();
}

void
Core::loop(void)
{
	double		lastTime, currentTime;
	double		frames;

	frames = 0.0;
	lastTime = glfwGetTime();
	glUseProgram(program);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix.val);
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
			glfwSetWindowTitle(window, (std::to_string((int)frames) + " fps").c_str());
			frames = 0.0;
			lastTime += 1.0;
		}
	}
}
