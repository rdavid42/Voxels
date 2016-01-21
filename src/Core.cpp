
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
	tex = new GLuint[1];
	tex[0] = loadTexture("resources/testground.bmp");
}

void
glErrorCallback(GLenum        source,
				GLenum        type,
				GLuint        id,
				GLenum        severity,
				GLsizei       length,
				const GLchar* message,
				GLvoid*       userParam)
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
		0.0f, 1.0f, 1.0f, 1.0f, 0.0f, //	6	22
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f  //	7	23
	};
	static GLushort const		voxelIndices[42] =
	{
		// floor
		12, 13, 15,
		12, 14, 15,
		// ceiling
		16, 17, 18,
		17, 18, 19,
		// back
		0,  1,  3,
		0,  3,  2,
		// left
		4,  6,  7,
		4,  5,  7,
		// right
		8,  10, 11,
		8,  9,  11,
		// front
		20, 21, 23,
		20, 23, 22
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
	multiplier = 0.0f;
	initVoxel();
	loadTextures();
	octree = new Link(-OCTREE_SIZE / 2, -OCTREE_SIZE / 2, -OCTREE_SIZE / 2, OCTREE_SIZE);
	octree->insert(0.0, 0.0, 0.0, 8, BLOCK);
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
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	octree->render(*this);
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
