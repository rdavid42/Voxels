
#include "Core.hpp"

int			main(void)
{
	Core	core;

	if (!core.init())
		return (0);
	core.loop();
	return (1);
}
