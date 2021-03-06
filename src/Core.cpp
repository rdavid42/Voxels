
#include "Core.hpp"

Core::Core(void)
{
}

Core::~Core(void)
{
	stopThreads();
	delete octree;
	glfwDestroyWindow(window);
	glfwTerminate();
	std::cerr << "done" << std::endl;
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

	core->camera.vangle -= ((ypos - core->windowHeight * 0.5f) * 0.05);
	core->camera.vangle =  core->camera.vangle < -89.0f ? -89.0f : (core->camera.vangle > 89.0f ? 89.0f : core->camera.vangle);
// 	if (core->camera.vangle > 89.0f)
// 		core->camera.vangle = 89.0f;
// 	if (core->camera.vangle < -89.0f)
// 		core->camera.vangle = -89.0f;
	core->camera.hangle -= ((xpos - core->windowWidth * 0.5f) * 0.05f);
	core->camera.hangle = fmod(core->camera.hangle, 360);
	glfwSetCursorPos(core->window, core->windowWidth * 0.5f, core->windowHeight * 0.5f);

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
	textureIndexLoc = glGetAttribLocation(program, "textureIndex");
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	checkGlError(__FILE__, __LINE__);
	return (texture);
}

GLuint
Core::loadTextureArrayFromAtlas(char const *filename)
{
	GLuint				texture;
	Bmp					bmp;

	if (!bmp.load(filename))
		return (printError("Failed to load bmp !", 0));
	texture = 0;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0); // maybe useless
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8,
				256, 256, 10,
				0, GL_RGB, GL_UNSIGNED_BYTE, 0); // allocate the texture array
	glPixelStorei(GL_UNPACK_ROW_LENGTH, bmp.width); // set the total atlas size in order to cut through it with glTexSubImage3D
	for (int x = 0; x < 10; ++x)
	{
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
						0, 0, x, // z (using x tex coord) is the `index` of the array
						256, 256, 1, // generate 1 subtexture of 256x256
						GL_RGB, GL_UNSIGNED_BYTE, bmp.data + (x * 256) * 3); // get the pointer on the texture data using very advanced arithmetics. (index * width * RGB)
		checkGlError(__FILE__, __LINE__);
	}
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	// glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	return (texture);
}

void
Core::loadTextures(void)
{
	tex = new GLuint[1];
	if (!(tex[0] = loadTextureArrayFromAtlas("resources/atlas.bmp")))
		exit(0);
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

inline bool
checkBlockObstructedUp(Chunk *chunk, Cube const &c, float const &deepestBlockSize)
{
	float		cx, cz;

	for (cx = c.getX(); cx < c.getX() + c.getS(); cx += deepestBlockSize)
		for (cz = c.getZ(); cz < c.getZ() + c.getS(); cz += deepestBlockSize)
			if (!chunk->search(cx, c.getY() + c.getS(), cz, BLOCK, false))
				return (false);
	return (true);
}

inline bool
checkBlockObstructedBottom(Chunk *chunk, Cube const &c, float const &deepestBlockSize)
{
	float		cx, cz;

	for (cx = c.getX(); cx < c.getX() + c.getS(); cx += deepestBlockSize)
		for (cz = c.getZ(); cz < c.getZ() + c.getS(); cz += deepestBlockSize)
			if (!chunk->search(cx, c.getY() - c.getS(), cz, BLOCK, false))
				return (false);
	return (true);
}

inline bool
checkBlockObstructedBack(Chunk *chunk, Cube const &c, float const &deepestBlockSize)
{
	float		cx, cy;

	for (cx = c.getX(); cx < c.getX() + c.getS(); cx += deepestBlockSize)
		for (cy = c.getY(); cy < c.getY() + c.getS(); cy += deepestBlockSize)
			if (!chunk->search(cx, cy, c.getZ() - c.getS(), BLOCK, false))
				return (false);
	return (true);
}

inline bool
checkBlockObstructedFront(Chunk *chunk, Cube const &c, float const &deepestBlockSize)
{
	float		cx, cy;

	for (cx = c.getX(); cx < c.getX() + c.getS(); cx += deepestBlockSize)
		for (cy = c.getY(); cy < c.getY() + c.getS(); cy += deepestBlockSize)
			if (!chunk->search(cx, cy, c.getZ() + c.getS(), BLOCK, false))
				return (false);
	return (true);
}

inline bool
checkBlockObstructedLeft(Chunk *chunk, Cube const &c, float const &deepestBlockSize)
{
	float		cy, cz;

	for (cy = c.getY(); cy < c.getY() + c.getS(); cy += deepestBlockSize)
		for (cz = c.getZ(); cz < c.getZ() + c.getS(); cz += deepestBlockSize)
			if (!chunk->search(c.getX() - c.getS(), cy, cz, BLOCK, false))
				return (false);
	return (true);
}

inline bool
checkBlockObstructedRight(Chunk *chunk, Cube const &c, float const &deepestBlockSize)
{
	float		cy, cz;

	for (cy = c.getY(); cy < c.getY() + c.getS(); cy += deepestBlockSize)
		for (cz = c.getZ(); cz < c.getZ() + c.getS(); cz += deepestBlockSize)
			if (!chunk->search(c.getX() + c.getS(), cy, cz, BLOCK, false))
				return (false);
	return (true);
}

void
Core::generateChunkMesh(Chunk *chunk, Octree *current) const // multithread
{
	Cube					currentCube;
	int						i;
	float					x, y, z, s;
	float					bt;
	float					sd;
	float const				deepestBlockSize = block_size[BLOCK_DEPTH]; // deepest block size

	if (!current)
		return ;
	if (current->getState() == BLOCK)
	{
		bt = static_cast<float>(current->getType());
		currentCube = current->getCube();
		x = currentCube.getX();
		y = currentCube.getY();
		z = currentCube.getZ();
		s = currentCube.getS();
		if (!checkBlockObstructedUp(chunk, currentCube, deepestBlockSize))
		{
			chunk->mesh.pushVertex({x,			y + s,		z,			0.0f * 4 * s,	0.0f * 4 * s, bt}); // 2
			chunk->mesh.pushVertex({x,			y + s,		z + s,		1.0f * 4 * s,	0.0f * 4 * s, bt}); // 6
			chunk->mesh.pushVertex({x + s,		y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, bt}); // 7
			chunk->mesh.pushVertex({x + s,		y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, bt}); // 7
			chunk->mesh.pushVertex({x + s,		y + s,		z,			0.0f * 4 * s,	1.0f * 4 * s, bt}); // 3
			chunk->mesh.pushVertex({x,			y + s,		z,			0.0f * 4 * s,	0.0f * 4 * s, bt}); // 2
		}
		sd = bt;
		if (bt == GRASS)
			sd = DIRT;
		if (!checkBlockObstructedBottom(chunk, currentCube, deepestBlockSize))
		{
			chunk->mesh.pushVertex({x,			y,			z,			0.0f * 4 * s,	1.0f * 4 * s, sd}); // 0
			chunk->mesh.pushVertex({x + s,		y,			z,			1.0f * 4 * s,	1.0f * 4 * s, sd}); // 1
			chunk->mesh.pushVertex({x + s,		y,			z + s,		1.0f * 4 * s,	0.0f * 4 * s, sd}); // 5
			chunk->mesh.pushVertex({x + s,		y,			z + s,		1.0f * 4 * s,	0.0f * 4 * s, sd}); // 5
			chunk->mesh.pushVertex({x,			y,			z + s,		0.0f * 4 * s,	0.0f * 4 * s, sd}); // 4
			chunk->mesh.pushVertex({x,			y,			z,			0.0f * 4 * s,	1.0f * 4 * s, sd}); // 0
		}
		sd = bt;
		if (bt == GRASS)
			sd = NONE; // side grass, to rename
		if (!checkBlockObstructedBack(chunk, currentCube, deepestBlockSize))
		{
			chunk->mesh.pushVertex({x,			y,			z,			0.0f * 4 * s,	0.0f * 4 * s, sd}); // 0
			chunk->mesh.pushVertex({x,			y + s,		z,			0.0f * 4 * s,	1.0f * 4 * s, sd}); // 2
			chunk->mesh.pushVertex({x + s,		y + s,		z,			1.0f * 4 * s,	1.0f * 4 * s, sd}); // 3
			chunk->mesh.pushVertex({x + s,		y + s,		z,			1.0f * 4 * s,	1.0f * 4 * s, sd}); // 3
			chunk->mesh.pushVertex({x + s,		y,			z,			1.0f * 4 * s,	0.0f * 4 * s, sd}); // 1
			chunk->mesh.pushVertex({x,			y,			z,			0.0f * 4 * s,	0.0f * 4 * s, sd}); // 0
		}
		if (!checkBlockObstructedFront(chunk, currentCube, deepestBlockSize))
		{
			chunk->mesh.pushVertex({x,			y,			z + s,		0.0f * 4 * s,	0.0f * 4 * s, sd}); // 4
			chunk->mesh.pushVertex({x + s,		y,			z + s,		1.0f * 4 * s,	0.0f * 4 * s, sd}); // 5
			chunk->mesh.pushVertex({x + s,		y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, sd}); // 7
			chunk->mesh.pushVertex({x + s,		y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, sd}); // 7
			chunk->mesh.pushVertex({x,			y + s,		z + s,		0.0f * 4 * s,	1.0f * 4 * s, sd}); // 6
			chunk->mesh.pushVertex({x,			y,			z + s,		0.0f * 4 * s,	0.0f * 4 * s, sd}); // 4
		}
		if (!checkBlockObstructedLeft(chunk, currentCube, deepestBlockSize))
		{
			chunk->mesh.pushVertex({x,			y,			z,			0.0f * 4 * s,	0.0f * 4 * s, sd}); // 0
			chunk->mesh.pushVertex({x,			y,			z + s,		1.0f * 4 * s,	0.0f * 4 * s, sd}); // 4
			chunk->mesh.pushVertex({x,			y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, sd}); // 6
			chunk->mesh.pushVertex({x,			y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, sd}); // 6
			chunk->mesh.pushVertex({x,			y + s,		z,			0.0f * 4 * s,	1.0f * 4 * s, sd}); // 2
			chunk->mesh.pushVertex({x,			y,			z,			0.0f * 4 * s,	0.0f * 4 * s, sd}); // 0
		}
		if (!checkBlockObstructedRight(chunk, currentCube, deepestBlockSize))
		{
			chunk->mesh.pushVertex({x + s,		y,			z,			0.0f * 4 * s,	0.0f * 4 * s, sd}); // 1
			chunk->mesh.pushVertex({x + s,		y + s,		z,			0.0f * 4 * s,	1.0f * 4 * s, sd}); // 3
			chunk->mesh.pushVertex({x + s,		y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, sd}); // 7
			chunk->mesh.pushVertex({x + s,		y + s,		z + s,		1.0f * 4 * s,	1.0f * 4 * s, sd}); // 7
			chunk->mesh.pushVertex({x + s,		y,			z + s,		1.0f * 4 * s,	0.0f * 4 * s, sd}); // 5
			chunk->mesh.pushVertex({x + s,		y,			z,			0.0f * 4 * s,	0.0f * 4 * s, sd}); // 1
		}
	}
	for (i = 0; i < CHD_MAX; ++i)
		generateChunkMesh(chunk, current->getChild(i));
}

void
Core::createTree(Chunk *chunk, int const &depth, float x, float y, float z) const
{
	float			tx, ty, tz;
	float			bSize;
	float			ly;

	bSize = this->block_size[(int)depth]; 

	for (ly = y; ly <= y + 2; ly += bSize)
	{
		chunk->insert(x, ly, z, depth, BLOCK, WOOD);
	}
	for (tx = x + 1; tx > x - 1.5; tx -= bSize)
	{
		for (ty = ly + 1; ty > ly - 1.5; ty -= bSize)
		{
			for (tz = z + 1; tz > z - 1.5; tz -= bSize)
			{
				if (pow(tx - x, 2) + pow(ty - ly, 2) + pow(tz - z, 2) < 1) 
				{
					chunk->insert(tx, ty, tz, depth, BLOCK, LEAF);
				}
			}

		}
	}
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
	noise->configs.emplace_back(5, 6, 0.2, 0.2, 1);		// Tree								//	6
	srandom(time(NULL));
	std::cerr	<< "octaves:     " << this->noise->configs.at(0).octaves << std::endl
				<< "frequency:   " << this->noise->configs.at(0).frequency << std::endl
				<< "lacunarity:  " << this->noise->configs.at(0).lacunarity << std::endl
				<< "amplitude:   " << this->noise->configs.at(0).amplitude << std::endl
				<< "persistence: " << this->noise->configs.at(0).persistence << std::endl;
}

void
Core::generateBlock3d(Chunk *chunk, float const &x, float const &y, float const &z, int const &depth, int const &ycap) const // multithread
{
	float						n;
	float						nstone;
	// float						ncoal;
	float						nx, nz, ny;
	float						ntree;
	float						dbSize;
	float						bSize;
	int							i;
	
	nx = chunk->getCube().getX() + x;
	ny = chunk->getCube().getY() + y;
	nz = chunk->getCube().getZ() + z;
	dbSize = this->block_size[depth] * 2;
	bSize = this->block_size[depth];
	ntree = noise->fractal(6, nx, 0, nz);

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
			{
				if (ntree > 0.3 && chunk->search(nx, ny + dbSize, nz) != NULL
				&&  chunk->search(nx, ny + dbSize, nz)->getState() == EMPTY)
					createTree(chunk, depth, nx, ny + bSize, nz);
				if ((octree->search(nx, ny + bSize, nz, EMPTY, 1) != NULL
				&&	 octree->search(nx, ny + bSize, nz)->getState() == EMPTY))
					chunk->insert(nx, ny, nz, depth, BLOCK, GRASS);
				else
					chunk->insert(nx, ny, nz, depth, BLOCK, DIRT);
			}
			else
			{
				if ((nstone > 0.75 && nstone < 0.76) || (nstone > 0.65 && nstone < 0.66))
					chunk->insert(nx, ny, nz, depth, BLOCK, COAL);
				else
				{
					if (chunk->getState() != CHUNK)
						std::cerr << "Error -> generateBlock3d(): " << typeid(chunk).name() << " of state " << chunk->getState() << std::endl;
					chunk->insert(nx, ny, nz, depth, BLOCK, STONE);
				}
			}
		}
	}
}

void
Core::generateBlock(Chunk *chunk, float const &x, float const &y, float const &z, int const &depth) const // multithread
{
	float						altitude;
	float						nx, nz;
	float						ntree;
	
	if (!chunk)
	{
		std::cerr << "Null chunk!" << std::endl;
		return ;
	}
	nx = chunk->getCube().getX() + x;
	nz = chunk->getCube().getZ() + z;
	altitude = 0.0f;
	ntree = noise->fractal(6, nx, y, nz);
	for (int i = 0; i < 10.0f; i++)
		altitude += noise->fractal(2, nx, y, nz);
	if (ntree > 0.3)
		createTree(chunk, depth, nx, altitude, nz);
	for (; altitude > -25.0f; altitude -= this->block_size[depth])
		  chunk->insert(nx, altitude, nz, depth, BLOCK, GRASS);
}

void
Core::processChunkSimplification(Chunk *chunk) // multithread
{
	float					x, y, z;
	float					cx, cy, cz;
	float const				bs = block_size[BLOCK_DEPTH];
	Octree					*tmp;

	cx = chunk->getCube().getX();
	cy = chunk->getCube().getY();
	cz = chunk->getCube().getZ();
	for (z = cz; z < cz + chunk_size; z += bs)
	{
		for (y = cy; y < cy + chunk_size; y += bs)
		{
			for (x = cx; x < cx + chunk_size; x += bs)
			{
				tmp = chunk->search(x, y, z, BLOCK, false);
				if (tmp)
				{
					tmp->getParent()->backwardSimplification();
				}
			}
		}
	}
}

void
Core::processChunkGeneration(Chunk *chunk) // multithread
{
	float						x, z, y;
	int							depth;

	if (chunk->getGenerated())
		return ;
	chunk->setGenerated(false);
	depth = BLOCK_DEPTH;
	if (chunk->getStopGenerating())
	{
		chunk->setRenderDone(true);
		chunk->setGenerated(true);
		chunk->setRemovable(true);
		return ;
	}
	for (z = 0.0f; z < this->chunk_size; z += this->block_size[depth])
	{
		for (x = 0.0f; x < this->chunk_size; x += this->block_size[depth])
		{
			for (y = this->chunk_size; y >= 0.0f; y -= this->block_size[depth])
			{
				if (chunk->getStopGenerating())
				{
					chunk->setRemovable(true);
					return ;
				}
				if (chunk)
					generateBlock3d(chunk, x, y, z, depth, 50);
				//generateBlock(chunk, x, 1.5, z, depth);
			}
		}
	}
	processChunkSimplification(chunk);
	generateChunkMesh(chunk, chunk);
	chunk->setGenerated(true);
	chunk->setRemovable(true);
}

// THREAD POOL

void *
Core::executeThread(int const &id) // multithread
{
	Chunk			*chunk;

	while (true)
	{
		// lock task queue and try to pick a task
		pthread_mutex_lock(&task_mutex[id]);
		is_task_locked[id] = true;

		// make the thread wait when the pool is empty
		while (this->pool_state != STOPPED && this->task_queue[id].empty())
			pthread_cond_wait(&this->task_cond[id], &this->task_mutex[id]);

		// stop the thread when the pool is destroyed
		if (this->pool_state == STOPPED)
		{
			// unlock to exit
			is_task_locked[id] = false;
			pthread_mutex_unlock(&task_mutex[id]);
			pthread_exit(0);
		}

		// pick task to process
		chunk = task_queue[id].front();
		if (chunk != 0 && chunk->getState() == CHUNK)
		{
			task_queue[id].pop_front();

			// unlock task queue
			is_task_locked[id] = false;
			pthread_mutex_unlock(&task_mutex[id]);

			// process task
			processChunkGeneration(chunk);
		}
		else
		{
			if (chunk)
				std::cerr << "Error -> executeThread(): " << typeid(chunk).name() << " of state " << chunk->getState() << std::endl;
			else
				std::cerr << "Null chunk!" << std::endl;
			is_task_locked[id] = false;
			pthread_mutex_unlock(&task_mutex[id]);
			task_queue[id].pop_front();
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
Core::getConcurrentThreads() const
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
			std::cerr << "[" << i << "] Thread created: " << std::hex << threads[i] << std::dec << std::endl;
	}
	return (1);
}

void
Core::addTask(Chunk *chunk, int const &id)
{
	// lock task queue
	pthread_mutex_lock(&task_mutex[id]);
	is_task_locked[id] = true;

	// push task in queue
	task_queue[id].push_front(chunk);

	// clear thread task queues if they exceed TASK_QUEUE_OVERFLOW
	while (task_queue[id].size() > TASK_QUEUE_OVERFLOW)
		task_queue[id].pop_back();
	// wake up a thread to process task
	pthread_cond_signal(&task_cond[id]);

	// unlock task queue
	is_task_locked[id] = false;
	pthread_mutex_unlock(&task_mutex[id]);
}

void
Core::generation(void)
{
	int							cx, cy, cz;
	int							id;
	unsigned int				min;
	int							i;
	Chunk						*chunk;

	// get new chunks inside rendering area and add them to generation queues
	for (cz = 0; cz < GEN_SIZE; ++cz)
	{
		for (cy = 0; cy < GEN_SIZE; ++cy)
		{
			for (cx = 0; cx < GEN_SIZE; ++cx)
			{
				if (chunks[cz][cy][cx] != NULL)
				{
					chunk = chunks[cz][cy][cx];
					if (!chunk->getGenerated() && !chunk->getGenerating())
					{
						id = 0;
						min = TASK_QUEUE_OVERFLOW;
						for (i = 0; i < pool_size; ++i)
						{
							if (task_queue[i].size() < min)
							{
								id = i;
								min = task_queue[i].size();
							}
						}
						chunk->setGenerating(true);
						addTask(chunk, id);
					}
					if (chunk->getGenerated() && !chunk->getRenderDone())
					{
						// wait for the chunk generation and allocate the opengl mesh
						// opengl functions cannot be called from a thread so we do it on the main thread (current OpenGL context)
						chunk->mesh.createGL(positionLoc, textureLoc, textureIndexLoc);
						chunk->setRenderDone(true);
						chunk->setGenerating(false);
						chunk->setStopGenerating(false);
						chunk->setRemovable(true);
					}
				}
			}
		}
	}
}

bool
Core::chunkInTaskPool(Chunk const *chunk) const
{
	int										i;
	std::deque<Chunk *>::const_iterator		it, ite;

	for (i = 0; i < pool_size; ++i)
	{
		if (task_queue[i].size() > 0)
		{
			it = task_queue[i].begin();
			ite = task_queue[i].end();
			while (it != ite)
			{
				if (*it == chunk)
					return (true);
				++it;
			}
		}
	}
	return (false);
}

Vec3<int>
Core::getChunksDirection(Chunk *central) const
{
	int			x, y, z;
	// get the vector between the current central chunk and the new one
	for (z = 0; z < GEN_SIZE; ++z)
		for (y = 0; y < GEN_SIZE; ++y)
			for (x = 0; x < GEN_SIZE; ++x)
				if (central == chunks[z][y][x])
					return (Vec3<int>(x - center, y - center, z - center));
	return (Vec3<int>(0, 0, 0));
}

void
Core::updateChunks(void)
{
	Chunk								*central;
	int									x, y, z;
	Vec3<int>							dir;
	Chunk								*newChunks[GEN_SIZE][GEN_SIZE][GEN_SIZE];
	std::list<Chunk *>::const_iterator	it;
	bool								inside;

	central = static_cast<Chunk *>(octree->search(camera.pos.x, camera.pos.y, camera.pos.z, CHUNK, false));
	if (central != chunks[center][center][center])
	{
		dir = getChunksDirection(central);
		if (dir.x == 0 && dir.y == 0 && dir.z == 0)
			return ; // outside of the generation area
		for (z = 0; z < GEN_SIZE; ++z)
			for (y = 0; y < GEN_SIZE; ++y)
				for (x = 0; x < GEN_SIZE; ++x)
					newChunks[z][y][x] = NULL;
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
					{
						if (chunks[z][y][x] != NULL)
						{
							// if (chunks[z][y][x]->getGenerating() && !chunks[z][y][x]->getGenerated())
								// chunks[z][y][x]->setStopGenerating(true);
							inside = false;
							for (it = chunksRemoval.begin(); it != chunksRemoval.end(); ++it)
							{
								if (*it == chunks[z][y][x])
								{
									inside = true;
									break;
								}
							}
							if (!inside)
								chunksRemoval.push_front(chunks[z][y][x]);
						}
					}
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
		insertChunks();
	}
}

void
Core::insertChunks(void)
{
	int									cx, cy, cz;
	float								px, py, pz;
	Chunk *								chunk;
	bool								inTaskPool;
	std::list<Chunk *>::iterator		it, ite;

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
						chunk = static_cast<Chunk *>(octree->insert(px, py, pz, CHUNK_DEPTH, CHUNK, NONE));
						if (chunk != 0)
						{
							inTaskPool = chunkInTaskPool(chunk);
							it = chunksRemoval.begin();
							ite = chunksRemoval.end();
							while (it != ite)
							{
								if (*it == chunk)
								{
									it = chunksRemoval.erase(it);
									break;
								}
								++it;
							}
							if (chunk != chunks[cz][cy][cx])
							{
								if (!inTaskPool)
								{
									chunk->setGenerated(false);
									chunk->setGenerating(false);
									chunk->setRenderDone(false);
									chunk->setStopGenerating(false);
									chunk->setRemovable(false);
								}
								else
								{
									std::cerr << chunk << "-> " << "state: " << (int)chunk->getGenerating() << (int)chunk->getGenerated() << (int)chunk->getRenderDone() << (int)chunk->getStopGenerating() << (int)chunk->getRemovable() << std::endl;
								}
								chunk->pos.x = cx;
								chunk->pos.y = cy;
								chunk->pos.z = cz;
								chunks[cz][cy][cx] = chunk;
							}
						}
					}
				}
			}
		}
	}
}

bool
Core::chunkInView(Chunk *chunk) const
{
	for (int z = 0; z < GEN_SIZE; ++z)
		for (int y = 0; y < GEN_SIZE; ++y)
			for (int x = 0; x < GEN_SIZE; ++x)
				if (chunks[z][y][x] == chunk)
					return (true);
	return (false);
}

void
Core::clearChunksRemoval(void)
{
	std::list<Chunk *>::iterator		it;
	Chunk *								chunk;

	if (chunksRemoval.size() > 0)
	{
		for (it = chunksRemoval.begin(); it != chunksRemoval.end();)
		{
			chunk = *it;
			// std::cerr << chunk << "-> " << "state: " << (int)chunk->getGenerating() << (int)chunk->getGenerated() << (int)chunk->getRenderDone() << (int)chunk->getStopGenerating() << (int)chunk->getRemovable() << std::endl;
			if (chunk && chunk->getRemovable())
			{
				if (!chunkInView(chunk))
				{
					chunk->remove();
					it = chunksRemoval.erase(it);
				}
				else
					it++;
			}
			else
				it++;
		}
	}
}

Block *
Core::getClosestBlock(void) const
{
	Vec3<float>			pos;
	int					i;
	int const			precision = 10;
	int const			dist = 10 * precision; // blocks max distance
	Block				*block;

	pos = camera.pos;
	for (i = 0; i < dist; ++i)
	{
		block = static_cast<Block *>(octree->search(pos.x, pos.y, pos.z, BLOCK, false));
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
	chunks[center][center][center]->setGenerated(false);
	chunks[center][center][center]->setGenerating(false);
	chunks[center][center][center]->setRenderDone(false);
	chunks[center][center][center]->setStopGenerating(false);
	chunks[center][center][center]->setRemovable(false);
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
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	float const fov = 53.13f;
	float const aspect = windowWidth * 1.0f / windowHeight;
	float const near = 0.1f;
	float const far = 1000.0f;
	buildProjectionMatrix(projMatrix, fov, near, far);
	camera.init(fov, aspect, near, far);
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
	frameRenderedTriangles = 0;
	std::cerr << "sizeof(Octree) = " << sizeof(Octree) << " bytes" << std::endl;
	std::cerr << "sizeof(Link)   = " << sizeof(Link) << " bytes" << std::endl;
	std::cerr << "sizeof(Block)  = " << sizeof(Block) << " bytes" << std::endl;
	return (1);
}

void
Core::updateLeftClick(void)
{
	Chunk			*chunk;

	if (closestBlock != NULL)
	{
		chunk = closestBlock->getChunk();
		closestBlock->remove();
		std::cerr << chunk->mesh.getPrimitives() << std::endl;
		chunk->mesh.clear();
		chunk->mesh.deleteGL();
		std::cerr << chunk->mesh.getPrimitives() << std::endl;
		generateChunkMesh(chunk, chunk);
		processChunkSimplification(chunk);
		std::cerr << chunk->mesh.getPrimitives() << std::endl;
		chunk->mesh.createGL(positionLoc, textureLoc, textureIndexLoc);
		chunk->mesh.clear();
		std::cerr << chunk->mesh.getPrimitives() << std::endl;
	}
}

void
Core::update(void)
{
	std::cerr << "remove list: " << chunksRemoval.size() << std::endl;
	glfwSetCursorPos(window, windowWidth / 2.0f, windowHeight / 2.0f);
	camera.rotate();
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
	updateChunks();
	if (chunksRemoval.size() > 0)
		clearChunksRemoval();
}

void
Core::render(void)
{
	float		ftime = glfwGetTime();
	int			x, y, z;
	size_t		t;

	(void)ftime;
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.view.val);
	ms.push();
		// render chunks ridges
		glBindVertexArray(selectionVao);
		glUniform1f(renderVoxelRidgesLoc, 1.0f);
		glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
/*		for (z = 0; z < GEN_SIZE; ++z)
		{
			for (y = 0; y < GEN_SIZE; ++y)
			{
				for (x = 0; x < GEN_SIZE; ++x)
				{
					if (chunks[z][y][x] != NULL)
						chunks[z][y][x]->renderRidges(*this);
				}
			}
		}*/
		if (closestBlock != NULL)
			closestBlock->renderRidges(*this);
		// render meshes
		glUniform1f(renderVoxelRidgesLoc, 0.0f);
		t = 0;
		for (z = 0; z < GEN_SIZE; ++z)
		{
			for (y = 0; y < GEN_SIZE; ++y)
			{
				for (x = 0; x < GEN_SIZE; ++x)
				{
					if (chunks[z][y][x] != NULL)
					{
						if (camera.cubeInFrustrum(chunks[z][y][x]->getCube()) == INSIDE)
						{
							chunks[z][y][x]->render(*this);
							t += chunks[z][y][x]->mesh.getPrimitives();
						}
					}
				}
			}
		}
		frameRenderedTriangles = t;
	ms.pop();
	glfwSwapBuffers(window);
}

void
Core::loop(void)
{
	double		lastTime, currentTime;
	double		frames;
	double		tmpTime;
	double		updateTime;
	double		renderTime;

	frames = 0.0;
	lastTime = glfwGetTime();
	glUseProgram(program);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix.val);
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		frames += 1.0;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		tmpTime = glfwGetTime();
		update();
		updateTime = glfwGetTime() - tmpTime;

		tmpTime = glfwGetTime();
		render();
		renderTime = glfwGetTime() - tmpTime;

		glfwPollEvents();
		if (currentTime - lastTime >= 1.0)
		{
			std::string timers = std::to_string(updateTime) + " / " + std::to_string(renderTime) + " / " + std::to_string(frameRenderedTriangles) + " triangles";
			glfwSetWindowTitle(window, (std::to_string((int)frames) + " fps [" + timers + "]").c_str());
			frames = 0.0;
			lastTime += 1.0;
		}
	}
}
