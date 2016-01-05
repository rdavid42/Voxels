
#include "Core.hpp"

Core::Core(void)
{

}

Core::~Core(void)
{

}

void
Core::checkGlError(std::string file, int line)
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
	glBindFragDataLocationEXT(program, 0, "out_fragment");
	if (!linkProgram(program))
		return (0);
	checkGlError(__FILE__, __LINE__);
	deleteShaders();
	return (1);
}

void
Core::init(int const &width, int const &height)
{
	sf::ContextSettings		settings(24, 8, 4, 4, 1);
	window.create(sf::VideoMode(width, height), "Voxels", sf::Style::Titlebar | sf::Style::Close, settings);
	sf::VideoMode			mode = sf::VideoMode::getDesktopMode();
	sf::ContextSettings		wsettings = window.getSettings();
	std::cerr << "OpenGL " << wsettings.majorVersion << "." << wsettings.minorVersion << std::endl;
	window.setPosition(sf::Vector2i(mode.width / 2 - width / 2, mode.height / 2 - height / 2));
	window.setVerticalSyncEnabled(true);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	initShaders();
}

void
Core::loop(void)
{
	sf::Event			event;

	while (true)
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				return;
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					return;
			}
			if (event.type == sf::Event::Resized)
			{
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		window.display();
	}
}

Core &
Core::operator=(Core const & rhs)
{
	if (this != &rhs)
	{
	}
	return (*this);
}
