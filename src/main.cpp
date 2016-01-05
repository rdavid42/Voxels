
#include <iostream>
#include "Core.hpp"

int
main(void)
{
	Core		core;

	core.init(1920, 1080);
	core.loop();
	return (0);
}