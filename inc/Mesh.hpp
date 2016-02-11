#ifndef MESH_HPP
# define MESH_HPP

# define STRIDE			5

# include <vector>
# include <iostream>

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

	std::vector<float>				data;

	Mesh(void);
	~Mesh(void);

	void							reserve(int const &size);
	void							clear(void);
	void							pushVertex(Vertex const &vertex);

	int								size(void) const;
	int const &						vertices(void) const;
	int const &						stride(void) const;

private:
	int								_vertices;
	static int						_stride;
};

#endif