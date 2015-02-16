
#include <iostream>
#include <thread>

void
test_func(void)
{
	std::cout << 10 << std::endl;
}

int
main(void)
{
	std::thread		t = std::thread(test_func);

	t.join();
	return (0);
}