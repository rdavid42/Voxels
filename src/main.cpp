
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include "Engine.hpp"

inline static void
check_sizes(void)
{
	std::cerr << "Octree size: " << sizeof(Octree) * CHAR_BIT << " bits, " << sizeof(Octree) << " bytes." << std::endl;
	std::cerr << "Cube size: " << sizeof(Cube) * CHAR_BIT << " bits, " << sizeof(Cube) << " bytes." << std::endl;
}

int
main(int argc, char **argv)
{
	Engine			engine;

	check_sizes();
	if (!engine.init())
		return (0);
	glutInit(&argc, argv);
	engine.loop();
	return (0);
}
