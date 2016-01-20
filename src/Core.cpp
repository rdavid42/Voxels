
#include "Core.hpp"

Core::Core(void)
{
}

Core::~Core(void)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void
checkGlError(std::string file, int line)
{
	GLenum		err;

	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		if (err == GL_INVALID_ENUM)
			std::cerr << "GL: Invalid enum in " << file << " line " << line << std::endl;
		else if (err == GL_INVALID_VALUE)
			std::cerr << "GL: Invalid value in " << file << " line " << line << std::endl;
		else if (err == GL_INVALID_OPERATION)
			std::cerr << "GL: Invalid operation in " << file << " line " << line << std::endl;
		else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
			std::cerr << "GL: Invalid framebuffer operation in " << file << " line " << line << std::endl;
		else if (err == GL_OUT_OF_MEMORY)
			std::cerr << "GL: Out of memory in " << file << " line " << line << std::endl;
	}
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

	// std::cerr << xpos << ", " << ypos << std::endl;
	// core->cameraRotate();
	core->vangle -= ((ypos - core->lastMy) * 0.05);
	if (core->vangle > 89)
		core->vangle = 89;
	if (core->vangle < -89)
		core->vangle = -89;
	core->hangle -= ((xpos - core->lastMx) * 0.05);
	core->hangle = fmod(core->hangle, 360);
	glfwSetCursorPos(core->window, core->windowWidth / 2, core->windowHeight / 2);
	//core->cameraRotate();
	core->lastMx = core->windowWidth / 2;
	core->lastMy = core->windowHeight / 2;
	(void)core;
	(void)xpos;
	(void)ypos;
}

/*
** SHADERS
*/

int
Core::compileShader(GLuint shader, char const *filename)
{
	GLint			logsize;
	GLint			state;
	char			*compileLog;

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
		compileLog = new char[logsize + 1];
		std::memset(compileLog, '\0', logsize + 1);
		glGetShaderInfoLog(shader, logsize, &logsize, compileLog);
		std::cerr	<< "Failed to compile shader `"
					<< filename
					<< "`: " << std::endl
					<< compileLog;
		delete compileLog;
		return (0);
	}
	return (1);
}

GLuint
Core::loadShader(GLenum type, char const *filename)
{
	GLuint			shader;
	char			*source;

	shader = glCreateShader(type);
	if (shader == 0)
		return (printError("Failed to create shader !", 0));
	if (!(source = readFile(filename)))
		return (printError("Failed to read file !", 0));
	glShaderSource(shader, 1, (char const **)&source, 0);
	if (!compileShader(shader, filename))
		return (0);
	delete source;
	return (shader);
}

int
Core::loadShaders(void)
{
	if (!(vertexShader = loadShader(GL_VERTEX_SHADER, "./shaders/vertex_shader.gls")))
		return (printError("Failed to load vertex shader !", 0));
	if (!(fragmentShader = loadShader(GL_FRAGMENT_SHADER, "./shaders/fragment_shader.gls")))
		return (printError("Failed to load fragment shader !", 0));
	return (1);
}

int
Core::linkProgram(GLuint &program)
{
	GLint			logSize;
	GLint			state;
	char			*linkLog;

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		linkLog = new char[logSize + 1];
		std::memset(linkLog, '\0', logSize + 1);
		glGetProgramInfoLog(program, logSize, &logSize, linkLog);
		std::cerr	<< "Failed to link program !" << std::endl
					<< linkLog;
		delete [] linkLog;
		return (0);
	}
	return (1);
}

void
Core::deleteShaders(void)
{
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

int
Core::initShaders(void)
{
	if (!loadShaders())
		return (0);
	if (!(program = glCreateProgram()))
		return (printError("Failed to create program !", 0));
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glBindFragDataLocation(program, 0, "out_fragment");
	if (!linkProgram(program))
		return (0);
	checkGlError(__FILE__, __LINE__);
	deleteShaders();
	return (1);
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
Core::setViewMatrix(Mat4<float> &view, Vec3<float> const &dir,
					Vec3<float> const &right, Vec3<float> const &up)
{
	/*
	rx		ux		-dx		0
	ry		uy		-dy		0
	rz		uz		-dz		0
	0		0		0		1
	*/
	// first column
	view[0] = right.x;
	view[4] = right.y;
	view[8] = right.z;
	view[12] = 0.0f;
	// second column
	view[1] = up.x;
	view[5] = up.y;
	view[9] = up.z;
	view[13] = 0.0f;
	// third column
	view[2] = -dir.x;
	view[6] = -dir.y;
	view[10] = -dir.z;
	view[14] = 0.0f;
	// fourth column
	view[3] = 0.0f;
	view[7] = 0.0f;
	view[11] = 0.0f;
	view[15] = 1.0f;
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
	cameraPos.set(15.0f, 15.0f, 15.0f);
	cameraLookAt.set(0.0f, 0.0f, 0.0f);
	initCamera();
	if (!initShaders())
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
	return (1);
}

void
Core::setCamera(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &forward)
{
	Mat4<float>		translation;

	cameraUp.set(0.0f, 1.0f, 0.0f);
	cameraRight.crossProduct(forward, cameraUp);
	cameraRight.normalize();
	cameraUp.crossProduct(cameraRight, forward);
	cameraUp.normalize();
	setViewMatrix(view, forward, cameraRight, cameraUp);
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
}

void
Core::initCamera(void)
{
	cameraPos.set(0.0f, 5.0f, 15.0f);
	cameraLookAt.set(0.0f, 0.0f, 0.0f);
	cameraForward.set(cameraLookAt - cameraPos);
	cameraForward.normalize();
	std::cerr << cameraForward << std::endl;
	cameraForward.normalize();
	// std::cerr << cameraForward << std::endl;
	setCamera(viewMatrix, cameraPos, cameraForward);
}
/*
void
Camera::onMouseMotion(SDL_MouseMotionEvent const &e)
{
	_theta -= e.xrel * _sensivity; 
	_phi -= e.yrel * _sensivity;
	this->calcVectors();
}

void
Camera::calcVectors(void)
{
	static const Vec3<float>	up(0.0f, 0.0f, 1.0f);
	double						rtmp;

	if (_phi > 89)
		_phi = 89;
	else if (_phi < -89)
		_phi = -89;
	rtmp = cos(_phi * M_PI / 180);
	_forward.x = rtmp * cos(_theta * M_PI / 180);
	_forward.y = rtmp * sin(_theta * M_PI / 180);
	_forward.z = sin(_phi * M_PI / 180);
	_left = up.crossProduct(_forward);
	_left.normalize();
	_forward.normalize();
	_target = _position + _forward;
}
*/
void
Core::cameraMoveForward(void)
{
	cameraPos += cameraForward;
	setCamera(viewMatrix, cameraPos, cameraForward);
}

void
Core::cameraMoveBackward(void)
{
	cameraPos -= cameraForward;
	setCamera(viewMatrix, cameraPos, cameraForward);
}

void
Core::cameraStrafeRight(void)
{
	cameraPos += cameraRight;
	setCamera(viewMatrix, cameraPos, cameraForward);
}

void
Core::cameraStrafeLeft(void)
{
	cameraPos -= cameraRight;
	setCamera(viewMatrix, cameraPos, cameraForward);
}

void
Core::cameraRotate(void)
{
	float			hr;
	float			vr;

	hr = hangle * M_PI / 180;
	vr = vangle * M_PI / 180;
	cameraForward.set(cos(vr) * sin(hr),
					 sin(vr),
					 cos(vr) * cos (hr));
	cameraForward.normalize();
	setCamera(viewMatrix, cameraPos, cameraForward);
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

void
Core::update(void)
{
	cameraRotate();
	setCamera(viewMatrix, cameraPos, cameraForward);
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
		cameraMoveForward();
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraMoveBackward();
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraStrafeLeft();
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraStrafeRight();
}

void
Core::render(void)
{
	float		ftime = glfwGetTime();

	(void)ftime;
	glUseProgram(program);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix.val);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix.val);
	glBindVertexArray(voxelVao);
	glBindBuffer(GL_ARRAY_BUFFER, voxelVbo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxelVbo[1]);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 5; k++)
			{
				ms.push();
					ms.translate((float)i + multiplier * (float)i, (float)k + multiplier * (float)k, (float)j + multiplier * (float)j);
					glUniformMatrix4fv(objLoc, 1, GL_FALSE, ms.top().val);
					glDrawElements(GL_TRIANGLES, 42, GL_UNSIGNED_SHORT, (void *)(sizeof(GLushort) * 0));
				ms.pop();
			}
		}
	}
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
