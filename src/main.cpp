
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include "Core.hpp"
#include "Biome.hpp"

inline static void
check_sizes(void)
{
	std::cerr << "Octree size: " << sizeof(Octree) * CHAR_BIT << " bits, " << sizeof(Octree) << " bytes." << std::endl;
	std::cerr << "Link size: " << sizeof(Link) * CHAR_BIT << " bits, " << sizeof(Link) << " bytes." << std::endl;
	std::cerr << "Biome size: " << sizeof(Biome) * CHAR_BIT << " bits, " << sizeof(Biome) << " bytes." << std::endl;
	std::cerr << "Chunk size: " << sizeof(Chunk) * CHAR_BIT << " bits, " << sizeof(Chunk) << " bytes." << std::endl;
	std::cerr << "Block size: " << sizeof(Block) * CHAR_BIT << " bits, " << sizeof(Block) << " bytes." << std::endl;
	std::cerr << "Cube size: " << sizeof(Cube) * CHAR_BIT << " bits, " << sizeof(Cube) << " bytes." << std::endl;
}

int
main(int argc, char **argv)
{
	Core			core;

	check_sizes();
	if (!core.init())
		return (0);
#ifdef linux // glutInit called automatically on Mac osx
	glutInit(&argc, argv);
#else
	(void)argc;
	(void)argv;
#endif
	core.loop();
	return (0);
}
