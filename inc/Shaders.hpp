#ifndef SHADERS_HPP
# define SHADERS_HPP

# include "Utils.hpp"

int				compileShader(GLuint shader, char const *filename);
GLuint			loadShader(GLenum type, char const *filename);
int				loadShaders(void);
int				linkProgram(GLuint &program);
void			deleteShaders(GLuint &vertexShader, GLuint &fragmentShader);
int				initShaders(GLuint &vertexShader, GLuint &fragmentShader, GLuint &program);

#endif