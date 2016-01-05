
#include <random>
#include "Utils.hpp"

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

int
slen(char *str)
{
	int			n;

	n = 0;
	while (str[n])
		n++;
	return (n);
}

float
getProb(void)
{
	return (((float)(random() % 100)) / 100.0f);
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
stoi(const char *str)
{
	int		res;
	int		i;
	int		neg;

	i = 0;
	res = 0;
	neg = 1;
	while (str[i] == ' ' || str[i] == '\f' || str[i] == '\n'
			|| str[i] == '\r' || str[i] == '\t' || str[i] == '\v')
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			neg = -1;
		i++;
	}
	while (str[i] <= '9' && str[i] >= '0')
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res * neg);
}

char *
scpy(char *s1, const char *s2)
{
	int		i;

	i = 0;
	while (s2[i] != 0)
	{
		s1[i] = s2[i];
		i++;
	}
	s1[i] = '\0';
	return (s1);
}

static size_t
getsize(int n)
{
	size_t			size;

	size = 0;
	if (n == 0)
	{
		size++;
		return (size);
	}
	if (n < 0)
	{
		n = -n;
		size++;
	}
	while (n > 0)
	{
		n = n / 10;
		size++;
	}
	return (size);
}

char *
itos(int n)
{
	char		*res;
	size_t		size;
	int			neg;

	if (n == -2147483648 && (res = (char *)malloc(12)))
	{
		scpy(res, "-2147483648");
		return (res);
	}
	size = getsize(n);
	neg = 0;
	if (n < 0 && (neg = 1))
		n = -n;
	res = (char *)malloc(size);
	res[size] = '\0';
	while (size)
	{
		size--;
		res[size] = (char)(n % 10 + 48);
		n = n / 10;
	}
	if (neg == 1)
		res[0] = '-';
	return (res);
}
