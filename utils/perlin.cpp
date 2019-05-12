#include "perlin.h"
#include <cmath>

using namespace utils;

utils::perlin_noise::perlin_noise()
{
	for (int x = 0; x < 512; x++)
	{
		perm2_[x] = perm_[x % 256];
	}
}


double perlin_noise::make_some_noise(double x, double y, double z, int octaves)
{
	double result = 0;
	double frequency = 1;
	double amplitude = 1;
	double max = 0;

	for (int i = 0; i < octaves; i++) //for each octave calculate perlin noise
	{
		result += perlin_(x * frequency, y * frequency, z * frequency) * amplitude;

		max += amplitude;

		amplitude *= 0.5;
		frequency *= 2;
	}

	return result / max;
}

double perlin_noise::perlin_(double x, double y, double z)
{

	double xdiff = x - (int)x;                             // store the diffs
	double ydiff = y - (int)y;
	double zdiff = z - (int)z;

	int xa = (int)x & 255;                              // compute unit cube
	int ya = (int)y & 255;
	int za = (int)z & 255;

	double u = fade_(xdiff);                             //smoothing results                             
	double v = fade_(ydiff);
	double w = fade_(zdiff);

	/*
	 * Perlin hash function
	 *  we get random value by looking at our position xa in the table perm2_
	 *  result + our ya is again looked in the table
	 *  and for the last time, we add za and look in the table
	 *  each next we add 1 when we look
	 *  at the end, we have 8 pseudorandom values between 0 - 255
	 */
	int aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = perm2_[perm2_[perm2_[xa] + ya] + za];
	aba = perm2_[perm2_[perm2_[xa] + ya + 1] + za];
	aab = perm2_[perm2_[perm2_[xa] + ya] + za + 1];
	abb = perm2_[perm2_[perm2_[xa] + ya + 1] + za + 1];
	baa = perm2_[perm2_[perm2_[xa + 1] + ya] + za];
	bba = perm2_[perm2_[perm2_[xa + 1] + ya + 1] + za];
	bab = perm2_[perm2_[perm2_[xa + 1] + ya] + za + 1];
	bbb = perm2_[perm2_[perm2_[xa + 1] + ya + 1] + za + 1];

	/*
	 * main part of the program, where we get our gradients
	 * we interpolate_ between new values with u and v respectively
	*/
	double x1, x2, y1, y2;
	x1 = interpolate_(gradient_(aaa, xdiff, ydiff, zdiff), gradient_(baa, xdiff - 1, ydiff, zdiff), u);
	x2 = interpolate_(gradient_(aba, xdiff, ydiff - 1, zdiff), gradient_(bba, xdiff - 1, ydiff - 1, zdiff), u);
	y1 = interpolate_(x1, x2, v);
	x1 = interpolate_(gradient_(aab, xdiff, ydiff, zdiff - 1), gradient_(bab, xdiff - 1, ydiff, zdiff - 1), u);
	x2 = interpolate_(gradient_(abb, xdiff, ydiff - 1, zdiff - 1), gradient_(bbb, xdiff - 1, ydiff - 1, zdiff - 1), u);
	y2 = interpolate_(x1, x2, v);

	return  (interpolate_(y1, y2, w) + 1) / 2;
}

double perlin_noise::gradient_(int hash, double x, double y, double z)
{
	double u, v;
	hash &= 15;                       //we take first 4 bits of hash

	if (hash < 8)
		u = x;           // highest bit is 0, u = x
	else
		u = y;                  // highest but is 1, u = y                       

	if (hash < 4)                             // highest and second highest bits are 0, v = y
		v = y;
	else if (hash == 12 || hash == 14)       // highest and second highest  bits are 1, v = x
		v = x;
	else                                                // highest and second highest are different, v = z
		v = z;

	return ((hash & 1) == 0 ? u : -u) + ((hash & 2) == 0 ? v : -v); // return addition of u and v with their respective sign to the last 2 bits
}

double perlin_noise::fade_(double t)
{
	return 6 * std::pow(t, 5) - 15 * std::pow(t, 4) + 10 * std::pow(t, 3);
}

double perlin_noise::interpolate_(double a, double b, double x)
{
	return x * (b - a) + a;
}