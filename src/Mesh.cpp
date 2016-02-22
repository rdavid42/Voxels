
#include "Mesh.hpp"
#include "Chunk.hpp"

int Mesh::_stride = sizeof(Vertex) / sizeof(float);

Mesh::Mesh(void) : _vao(0), _vbo(0), _mode(GL_TRIANGLES), _vertices(0)
{
	return ;
}

Mesh::~Mesh(void)
{
	clear();
	deleteGL();
	_vertices = -1;
}

void
Mesh::reserve(int const &n)
{
	_data.reserve(n);
}

void
Mesh::clear(void)
{
	_data.clear();
	std::vector<float>().swap(_data);
}

void
Mesh::createGL(GLuint const &positionLoc, GLuint const &textureLoc, GLuint const &textureIndexLoc)
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * getSize(), &_data[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * _stride, (void *)0);
	glEnableVertexAttribArray(textureLoc);
	glVertexAttribPointer(textureLoc, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * _stride, (void *)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(textureIndexLoc);
	glVertexAttribPointer(textureIndexLoc, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * _stride, (void *)(sizeof(GLfloat) * 5));
}

void
Mesh::deleteGL(void)
{
	if (glIsBuffer(_vbo))
		glDeleteBuffers(1, &_vbo);
	if (glIsVertexArray(_vao))
		glDeleteVertexArrays(1, &_vao);
	_vao = 0;
	_vbo = 0;
	_vertices = 0;
}

void
Mesh::pushVertex(Vertex const &vertex)
{
	_data.push_back(vertex.x);
	_data.push_back(vertex.y);
	_data.push_back(vertex.z);
	_data.push_back(vertex.tx);
	_data.push_back(vertex.ty);
	_data.push_back(vertex.ti);
	_vertices++;
}

//          y
//		    2----3
//		   /|   /|
//		 6----7  |
//		 |  0-|--1 x
//		 |/   | /
//		 4____5
//		z

void
Mesh::pushUpFace(float const &x, float const &y, float const &z, float const &s, float const &tex)
{
	pushVertex({x,			y + s,		z,			0.0f,	0.0f, tex}); // 2
	pushVertex({x,			y + s,		z + s,		1.0f,	0.0f, tex}); // 6
	pushVertex({x + s,		y + s,		z + s,		1.0f,	1.0f, tex}); // 7
	pushVertex({x + s,		y + s,		z + s,		1.0f,	1.0f, tex}); // 7
	pushVertex({x + s,		y + s,		z,			0.0f,	1.0f, tex}); // 3
	pushVertex({x,			y + s,		z,			0.0f,	0.0f, tex}); // 2
}

void
Mesh::pushBottomFace(float const &x, float const &y, float const &z, float const &s, float const &tex)
{
	pushVertex({x,			y,			z,			0.0f,	1.0f, tex}); // 0
	pushVertex({x + s,		y,			z,			1.0f,	1.0f, tex}); // 1
	pushVertex({x + s,		y,			z + s,		1.0f,	0.0f, tex}); // 5
	pushVertex({x + s,		y,			z + s,		1.0f,	0.0f, tex}); // 5
	pushVertex({x,			y,			z + s,		0.0f,	0.0f, tex}); // 4
	pushVertex({x,			y,			z,			0.0f,	1.0f, tex}); // 0
}

void
Mesh::pushQuad(int const &type, float const &x, float const &y, float const &z, float const &sx, float const &sy, float const &sz, float const &tex)
{
	if (type == Chunk::V_UP) // up, counter clockwise
	{
		pushVertex({x,			y + sy,		z,			0.0f,	0.0f, tex}); // 2
		pushVertex({x,			y + sy,		z + sz,		1.0f * (sz / BLOCK_SIZE),	0.0f, tex}); // 6
		pushVertex({x + sx,		y + sy,		z + sz,		1.0f * (sz / BLOCK_SIZE),	1.0f * (sx / BLOCK_SIZE), tex}); // 7
		pushVertex({x + sx,		y + sy,		z + sz,		1.0f * (sz / BLOCK_SIZE),	1.0f * (sx / BLOCK_SIZE), tex}); // 7
		pushVertex({x + sx,		y + sy,		z,			0.0f,	1.0f * (sx / BLOCK_SIZE), tex}); // 3
		pushVertex({x,			y + sy,		z,			0.0f,	0.0f, tex}); // 2
	}
	else if (type == Chunk::V_BOTTOM) // bottom, clockwise
	{
		pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
		pushVertex({x + sx,		y,			z,			0.0f,	1.0f * (sx / BLOCK_SIZE), tex}); // 1
		pushVertex({x + sx,		y,			z + sz,		1.0f * (sz / BLOCK_SIZE),	1.0f * (sx / BLOCK_SIZE), tex}); // 5
		pushVertex({x + sx,		y,			z + sz,		1.0f * (sz / BLOCK_SIZE),	1.0f * (sx / BLOCK_SIZE), tex}); // 5
		pushVertex({x,			y,			z + sz,		1.0f * (sz / BLOCK_SIZE),	0.0f, tex}); // 4
		pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
	}
}

void
Mesh::pushUpFace(float const &x, float const &y, float const &z, float const &sx, float const &sy, float const &sz, float const &tex)
{
	pushVertex({x,			y + sy,		z,			0.0f,	0.0f, tex}); // 2
	pushVertex({x,			y + sy,		z + sz,		1.0f * sz,	0.0f, tex}); // 6
	pushVertex({x + sx,		y + sy,		z + sz,		1.0f * sz,	1.0f * sx, tex}); // 7
	pushVertex({x + sx,		y + sy,		z + sz,		1.0f * sz,	1.0f * sx, tex}); // 7
	pushVertex({x + sx,		y + sy,		z,			0.0f,	1.0f * sx, tex}); // 3
	pushVertex({x,			y + sy,		z,			0.0f,	0.0f, tex}); // 2
}

void
Mesh::pushBottomFace(float const &x, float const &y, float const &z, float const &sx, float const &sy, float const &sz, float const &tex)
{
	(void)sy;
	pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
	pushVertex({x + sx,		y,			z,			0.0f,	1.0f * sx, tex}); // 1
	pushVertex({x + sx,		y,			z + sz,		1.0f * sz,	1.0f * sx, tex}); // 5
	pushVertex({x + sx,		y,			z + sz,		1.0f * sz,	1.0f * sx, tex}); // 5
	pushVertex({x,			y,			z + sz,		1.0f * sz,	0.0f, tex}); // 4
	pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
}

void
Mesh::pushBackFace(float const &x, float const &y, float const &z, float const &s, float const &tex)
{
	pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
	pushVertex({x,			y + s,		z,			0.0f,	1.0f, tex}); // 2
	pushVertex({x + s,		y + s,		z,			1.0f,	1.0f, tex}); // 3
	pushVertex({x + s,		y + s,		z,			1.0f,	1.0f, tex}); // 3
	pushVertex({x + s,		y,			z,			1.0f,	0.0f, tex}); // 1
	pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
}

void
Mesh::pushFrontFace(float const &x, float const &y, float const &z, float const &s, float const &tex)
{
	pushVertex({x,			y,			z + s,		0.0f,	0.0f, tex}); // 4
	pushVertex({x + s,		y,			z + s,		1.0f,	0.0f, tex}); // 5
	pushVertex({x + s,		y + s,		z + s,		1.0f,	1.0f, tex}); // 7
	pushVertex({x + s,		y + s,		z + s,		1.0f,	1.0f, tex}); // 7
	pushVertex({x,			y + s,		z + s,		0.0f,	1.0f, tex}); // 6
	pushVertex({x,			y,			z + s,		0.0f,	0.0f, tex}); // 4
}

void
Mesh::pushLeftFace(float const &x, float const &y, float const &z, float const &s, float const &tex)
{
	pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
	pushVertex({x,			y,			z + s,		1.0f,	0.0f, tex}); // 4
	pushVertex({x,			y + s,		z + s,		1.0f,	1.0f, tex}); // 6
	pushVertex({x,			y + s,		z + s,		1.0f,	1.0f, tex}); // 6
	pushVertex({x,			y + s,		z,			0.0f,	1.0f, tex}); // 2
	pushVertex({x,			y,			z,			0.0f,	0.0f, tex}); // 0
}

void
Mesh::pushRightFace(float const &x, float const &y, float const &z, float const &s, float const &tex)
{
	pushVertex({x + s,		y,			z,			0.0f,	0.0f, tex}); // 1
	pushVertex({x + s,		y + s,		z,			0.0f,	1.0f, tex}); // 3
	pushVertex({x + s,		y + s,		z + s,		1.0f,	1.0f, tex}); // 7
	pushVertex({x + s,		y + s,		z + s,		1.0f,	1.0f, tex}); // 7
	pushVertex({x + s,		y,			z + s,		1.0f,	0.0f, tex}); // 5
	pushVertex({x + s,		y,			z,			0.0f,	0.0f, tex}); // 1
}

int
Mesh::getSize(void) const
{
	return (_data.size());
}

int const &
Mesh::getStride(void) const
{
	return (_stride);
}

int const &
Mesh::getVertices(void) const
{
	return (_vertices);
}

GLenum const &
Mesh::getMode(void) const
{
	return (_mode);
}

GLuint const &
Mesh::getVAO(void) const
{
	return (_vao);
}

GLuint const &
Mesh::getVBO(void) const
{
	return (_vbo);
}

GLuint
Mesh::getPrimitives(void) const
{
	if (_mode == GL_TRIANGLES)
		return (_vertices / 3);
	else if (_mode == GL_QUADS)
		return (_vertices / 4);
	else if (_mode == GL_TRIANGLE_FAN)
		return (_vertices / 4);
	else if (_mode == GL_TRIANGLE_STRIP)
		return (_vertices / 4);
	return (_vertices);
}

void
Mesh::setMode(GLenum const &mode)
{
	_mode = mode;
}