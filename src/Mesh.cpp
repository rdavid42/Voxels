
#include "Mesh.hpp"

int Mesh::_stride = sizeof(Mesh::Vertex);

Mesh::Mesh(void)
{
	return ;
}

Mesh::~Mesh(void)
{
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
}

int
Mesh::size(void)
{
	return (data.size());
}

int const &
Mesh::stride(void)
{
	return (_stride);
}
