
__kernel void generateBlock(__global float *p)
{
	int const	i = get_global_id(0);

	(void)i;
	(void)p;
}
