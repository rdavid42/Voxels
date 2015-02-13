
#include "Noise.hpp"

Noise::Noise(uint32_t const &arg_seed, uint32_t const &arg_prm_size)
	: seed(arg_seed), prm_size(arg_prm_size)
{
	uint32_t		i;
	int				swap_i;
	int				tmp;
	uint32_t const	total = this->prm_size * 2;

	srandom(seed);
	this->prm = new int[total];
	for (i = 0; i < this->prm_size; ++i)
		this->prm[i] = i;
	for (i = 0; i < this->prm_size; ++i)
	{
		swap_i = random() % this->prm_size;
		tmp = this->prm[swap_i];
		this->prm[swap_i] = this->prm[i];
		this->prm[i] = tmp;
	}
	for (i = this->prm_size; i < total; ++i)
		this->prm[i] = this->prm[i - this->prm_size];
}

Noise::Config::Config(int layers, float frequency, float lacunarity, float amplitude, float gain)
	: layers(layers), frequency(frequency), lacunarity(lacunarity), amplitude(amplitude), gain(gain)
{
	return ;
}

Noise::~Noise(void)
{
	delete [] this->prm;
}

float
Noise::fade(float const &t)
{
	return (t * t * t * (t * (t * 6 - 15) + 10));
}

float
Noise::lerp(float const &t, float const &a, float const &b)
{
	return (a + t * (b - a));
}

float
Noise::grad(int const hash, float const &x, float const &y, float const &z)
{
	int							h;
	float						u;
	float						v;
	float						r;

	h = hash & 15;
	r = h == 12 || h == 14 ? x : z;
	u = h < 8 ? x : y;
	v = h < 4 ? y : r;
	if ((h & 1) == 0)
		r = u;
	else
		r = -u;
	if ((h & 2) == 0)
		r += v;
	else
		r += -v;
	return (r);
}

float
Noise::perlin(float x, float y, float z)
{
	Vec3<int>			v;
	Vec3<float>			u;
	Vec3<int>			a;
	Vec3<int>			b;

	v.set(static_cast<int>(floorf(x)) & 255,
		static_cast<int>(floorf(y)) & 255,
		static_cast<int>(floorf(z)) & 255);
	x -= floorf(x);
	y -= floorf(y);
	z -= floorf(z);
	u.set(fade(x), fade(y), fade(z));
	a.x = prm[v.x] + v.y;
	a.y = prm[a.x] + v.z;
	a.z = prm[a.x + 1] + v.z;
	b.x = prm[v.x + 1] + v.y;
	b.y = prm[b.x] + v.z;
	b.z = prm[b.x + 1] + v.z;
	return (lerp(u.z, lerp(u.y, lerp(u.x, grad(prm[a.y], x, y, z),
			grad(prm[b.y], x - 1, y, z)),
			lerp(u.x, grad(prm[a.z], x, y - 1, z),
			grad(prm[b.z], x - 1, y - 1, z))),
			lerp(u.y, lerp(u.x, grad(prm[a.y + 1], x, y, z - 1),
			grad(prm[b.y + 1], x - 1, y, z - 1)),
			lerp(u.x, grad(prm[a.z + 1], x, y - 1, z - 1),
			grad(prm[b.z + 1], x - 1, y - 1, z - 1)))));
}

float
Noise::fractal(int const ci, float const &x, float const &y, float const &z)
{
	float				fnoise;
	float				max_sum;
	float				frequency;
	float				amplitude;
	int					i;
	Config				*n = &this->configs.at(ci);

	fnoise = 0;
	max_sum = 0;
	frequency = n->frequency;
	amplitude = n->amplitude;
	for (i = 0; i < n->layers; ++i)
	{
		max_sum += amplitude;
		fnoise += this->perlin(x * frequency, y * frequency, z * frequency) * amplitude;
		frequency *= n->lacunarity;
		amplitude *= n->gain;
	}
	return (fnoise / max_sum);
}

Noise
&Noise::operator=(Noise const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream
&operator<<(std::ostream &o, Noise const &i)
{
	o	<< "Noise: " << &i;
	return (o);
}
