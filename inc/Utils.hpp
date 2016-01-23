#ifndef UTILS_HPP
# define UTILS_HPP

# include <cstring>
# include <ostream>
# include <sys/stat.h>
# include <iostream>
# include <fstream>
# include <ostream>
# include <fcntl.h>
# include <unistd.h>
# include <sstream>

# include "Constants.hpp"

# define BUFSIZE	4096

char *				readFile(char const *filename);
float				getProb(void);
int					printError(std::ostream &msg, int const &code);
int					printError(std::string const &msg, int const &code);
void *				printError(std::string const &msg);
std::string			getFileContents(std::string const &filename);
void				checkGlError(std::string file, int line);

#endif