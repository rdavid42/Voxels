
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
		int				octaves;
		float			frequency;
		float			lacunarity;
		float			amplitude;
		float			persistence;

		Config(int octaves, float frequency, float lacunarity, float amplitude, float persistence);
	};

	uint32_t const				seed;
	uint32_t const				prm_size;
	int							*prm; // permutation table
	std::vector<Config>			configs;

	Noise(uint32_t const &seed, uint32_t const &prm_size);
	virtual ~Noise(void);

	float				perlin(float x, float y, float z);
	float				fractal(int const &ci, float const &x, float const &y, float const &z);

	float				raw_noise_3d(float const &x, float const &y, float const &z);
	float				octave_noise_3d(int const &ci, float const &x, float const &y, float const &z);
	float				scaled_raw_noise_3d(float const &loBound, float const &hiBound, float const &x, float const &y, float const &z);
	float				scaled_octave_noise_3d(int const &ci, float const &loBound, float const &hiBound, float const &x, float const &y, float const &z);

	Noise &				operator=(Noise const &rhs);

private:
	float				fade(float const &t);
	float				lerp(float const &t, float const &a, float const &b);
	float				grad(int const hash, float const &x, float const &y, float const &z);

	Noise(Noise const &src);
};

std::ostream &			operator<<(std::ostream &o, Noise const &i);

#endif
