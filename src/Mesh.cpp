
#include "Mesh.hpp"

int Mesh::_stride = sizeof(Mesh::Vertex) / sizeof(float);

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
	return (_vertices);
}

void
Mesh::setMode(GLenum const &mode)
{
	_mode = mode;
}