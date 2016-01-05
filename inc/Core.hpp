#ifndef CORE_HPP
# define CORE_HPP

# include <iostream>
# include <SFML/window.hpp>
# include <SFML/system.hpp>
# include <SFML/OpenGL.hpp>
# include "Utils.hpp"

class Core
{
public:
	sf::Window				window;

	GLuint					vertexShader;
	GLuint					fragmentShader;
	GLuint					program;

	Core(void);
	~Core(void);

	void					checkGlError(std::string file, int line);

	/* shaders */
	void					getLocations(void);
	int						compileShader(GLuint shader, char const *filename);
	GLuint					loadShader(GLenum type, char const *filename);
	int						loadShaders(void);
	int						linkProgram(GLuint &p);
	void					deleteShaders(void);
	int						initShaders(void);

	/* core */
	void					init(int const &width, int const &height);
	void					loop(void);

	Core & operator = (Core const & rhs);
private:
	Core(Core const &src);
};

#endif