
#include "Utils.hpp"

void
checkGlError(std::string file, int line)
{
	GLenum		err;

	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		if (err == GL_INVALID_ENUM)
			std::cerr << "GL: Invalid enum in " << file << " line " << line << std::endl;
		else if (err == GL_INVALID_VALUE)
			std::cerr << "GL: Invalid value in " << file << " line " << line << std::endl;
		else if (err == GL_INVALID_OPERATION)
			std::cerr << "GL: Invalid operation in " << file << " line " << line << std::endl;
		else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
			std::cerr << "GL: Invalid framebuffer operation in " << file << " line " << line << std::endl;
		else if (err == GL_OUT_OF_MEMORY)
			std::cerr << "GL: Out of memory in " << file << " line " << line << std::endl;
	}
}

char *
readFile(char const *filename)
{
	struct stat		file_stat;
	int				fd;
	int				i;
	int				j;
	int				ret;
	char			buf[BUFSIZE];
	char			*file;

	if ((fd = open(filename, O_RDONLY)) == -1)
		return ((char *)printError("Failed to open file!"));
	if (fstat(fd, &file_stat) == -1)
		return ((char *)printError("Failed to retrieve file stat!"));
	file = new char[file_stat.st_size + 1];
	i = 0;
	while ((ret = read(fd, buf, BUFSIZE)) != 0)
	{
		if (ret == -1)
			return (close(fd), (char *)printError("Failed to read file!"));
		j = 0;
		while (j < ret)
		{
			file[i + j] = buf[j];
			j++;
		}
		i += ret;
	}
	file[i] = '\0';
	close(fd);
	return (file);
}

std::string
getFileContents(std::string const &filename)
{
	std::ifstream		in(filename, std::ios::in | std::ios::binary);
	std::string			contents;

	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();
	return (contents);
}

int
printError(std::ostream &msg, int const &code)
{
	std::cerr << dynamic_cast<std::ostringstream &>(msg).str() << std::endl;
	return (code);
}

int
printError(std::string const &msg, int const &code)
{
	std::cerr << msg << std::endl;
	return (code);
}

void *
printError(std::string const &msg)
{
	std::cerr << msg << std::endl;
	return (0);
}
