
#ifndef		NOISE_HPP
# define	NOISE_HPP

# include <iostream>
# include <stdint.h>
# include <stdlib.h>
# include <vector>
# include "Vec3.hpp"

class Noise
{
public:
	class				Config
	{
	public:
		int				layers;
		float			frequency;
		float			lacunarity;
		float			amplitude;
		float			gain;

		Config(int layers, float frequency, float lacunarity, float amplitude, float gain);
	};

	uint32_t const				seed;
	uint32_t const				prm_size;
	int							*prm; // permutation table
	std::vector<Config>			configs;

	Noise(uint32_t const &seed, uint32_t const &prm_size);
	virtual ~Noise(void);

	float				perlin(float x, float y, float z);
	float				fractal(int const ci, float const &x, float const &y, float const &z);

	Noise				&operator=(Noise const &rhs);

private:
	float				fade(float const &t);
	float				lerp(float const &t, float const &a, float const &b);
	float				grad(int const hash, float const &x, float const &y, float const &z);

	Noise(Noise const &src);
};

std::ostream			&operator<<(std::ostream &o, Noise const &i);

#endif
