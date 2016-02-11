
#include "Mesh.hpp"

int Mesh::_stride = sizeof(Mesh::Vertex);

Mesh::Mesh(void) : _vertices(0)
{
	return ;
}

Mesh::~Mesh(void)
{
	_vertices = -1;
}

void
Mesh::reserve(int const &n)
{
	data.reserve(n);
}

void
Mesh::clear(void)
{
	data.clear();
	std::vector<float>().swap(data);
}

void
Mesh::pushVertex(Vertex const &vertex)
{
	data.push_back(vertex.x);
	data.push_back(vertex.y);
	data.push_back(vertex.z);
	data.push_back(vertex.tx);
	data.push_back(vertex.ty);
	_vertices++;
}

int
Mesh::size(void) const
{
	return (data.size());
}

int const &
Mesh::stride(void) const
{
	return (_stride);
}

int const &
Mesh::vertices(void) const
{
	return (_vertices);
}
