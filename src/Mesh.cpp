
#include "Mesh.hpp"

int Mesh::_stride = sizeof(Mesh::Vertex);

Mesh::Mesh(void) : _data(0), _capacity(0), _size(0)
{
	return ;
}

Mesh::~Mesh(void)
{
	if (_size > 0)
	{
		for (int i = 0; i < _size; ++i)
			_data[i] = 0.0f;
		_size = 0;
		_capacity = 0;
		delete [] _data;
	}
}

void
Mesh::reserve(int const &n)
{
	float		*data;
	int			i;

	data = 0;
	if (n > 0 && _capacity != n)
	{
		if (_size > 0)
		{
			_capacity = n;
			data = new float[_capacity];
			if (_size > _capacity)
				_size = _capacity;
			for (i = 0; i < _size; ++i)
				data[i] = _data[i];
			delete [] _data;
			_data = data;
		}
		else
		{
			_size = n;
			_capacity = n;
			_data = new float[_capacity];
		}
	}
}

void
Mesh::clear(void)
{
	if (_size > 0)
	{
		_size = 0;
		_capacity = 0;
		delete [] _data;
	}
}

void
Mesh::push_back(Vertex const &vertex)
{
	int			it;

	it = _size;
	(void)vertex;
	_size += _stride;
	if (_size > _capacity)
		reserve(2 * _size);
	_data[it + 0] = vertex.x;
	_data[it + 1] = vertex.y;
	_data[it + 2] = vertex.z;
	_data[it + 3] = vertex.tx;
	_data[it + 4] = vertex.ty;
}

float const *
Mesh::get(void)
{
	return (_data);
}

int const &
Mesh::getSize(void)
{
	return (_size);
}

int const &
Mesh::getCapacity(void)
{
	return (_capacity);
}

int const &
Mesh::getStride(void)
{
	return (_stride);
}