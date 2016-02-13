#ifndef MESH_HPP
# define MESH_HPP

# define STRIDE			5

# include <vector>
# include <iostream>
# include "Constants.hpp"

class Mesh
{
public:
	struct Vertex
	{
		float			x;
		float			y;
		float			z;
		float			tx;
		float			ty;
	};

	Mesh(void);
	~Mesh(void);

	void							reserve(int const &size);
	void							clear(void);
	void							createGL(GLuint const &positionLoc, GLuint const &textureLoc);
	void							deleteGL(void);
	void							pushVertex(Vertex const &vertex);

	int								getSize(void) const;
	int const &						getVertices(void) const;
	int const &						getStride(void) const;
	GLenum const &					getMode(void) const;
	GLuint const &					getVAO(void) const;
	GLuint const &					getVBO(void) const;
	GLuint							getPrimitives(void) const;

	void							setMode(GLenum const &mode);

private:
	GLuint							_vao;
	GLuint							_vbo;
	GLenum							_mode;
	int								_vertices;

	static int						_stride;
	std::vector<float>				_data;
};

#endif