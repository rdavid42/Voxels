#ifndef MESH_HPP
# define MESH_HPP

# define STRIDE			5

# include <vector>
# include <iostream>
# include "Constants.hpp"

typedef struct
{
	float			x;
	float			y;
	float			z;
	float			tx;
	float			ty;
	float			ti;
}					Vertex;

class Mesh
{
public:
	Mesh(void);
	~Mesh(void);

	void							reserve(int const &size);
	void							clear(void);
	void							createGL(GLuint const &positionLoc, GLuint const &textureLoc, GLuint const &textureIndexLoc);
	void							deleteGL(void);
	void							pushVertex(Vertex const &vertex);
	void							pushQuad(int const &type, float const &x, float const &y, float const &z, float const &sx, float const &sy, float const &sz, float const &tex);
	void							pushUpFace(float const &x, float const &y, float const &z, float const &s, float const &tex);
	void							pushUpFace(float const &x, float const &y, float const &z, float const &sx, float const &sy, float const &sz, float const &tex);
	void							pushBottomFace(float const &x, float const &y, float const &z, float const &s, float const &tex);
	void							pushBottomFace(float const &x, float const &y, float const &z, float const &sx, float const &sy, float const &sz, float const &tex);
	void							pushBackFace(float const &x, float const &y, float const &z, float const &s, float const &tex);
	void							pushFrontFace(float const &x, float const &y, float const &z, float const &s, float const &tex);
	void							pushLeftFace(float const &x, float const &y, float const &z, float const &s, float const &tex);
	void							pushRightFace(float const &x, float const &y, float const &z, float const &s, float const &tex);

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