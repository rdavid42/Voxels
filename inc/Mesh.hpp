#ifndef MESH_HPP
# define MESH_HPP

# define STRIDE			5

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
	Mesh(int const &size);
	~Mesh(void);

	void			reserve(int const &size);
	void			clear(void);
	void			push_back(Vertex const &vertex);

	float const *	get(void);
	int const &		getSize(void);
	int const &		getCapacity(void);
	int const &		getStride(void);

private:
	float *			_data;
	int				_capacity;
	int				_size;

	static int		_stride;
};

#endif