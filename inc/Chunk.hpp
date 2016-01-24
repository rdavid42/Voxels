
#ifndef CHUNK_HPP
# define CHUNK_HPP

# include <iostream>
# include <vector>
# include "Constants.hpp"
# include "Link.hpp"

class Chunk : public Link
{
public:
	std::vector<GLfloat>	mesh;
	GLuint					vao;
	GLuint					vbo;
	int						meshSize;
	bool					generating;
	bool					generated;
	bool					renderDone;

	Vec3<uint8_t>	pos; // position relative to other chunks

	Chunk(void);
	Chunk(float const &x, float const &y, float const &z, float const &s);
	virtual ~Chunk(void);

	void				createMesh(void);
	virtual	void		render(Core &core) const;

	Chunk				&operator=(Chunk const &rhs);

private:
	Chunk(Chunk const &src);
};

std::ostream				&operator<<(std::ostream &o, Chunk const &i);

#endif
