#include "noise.h"

Noise::Noise(int s, double frequency, int octaves) : fastNoise(s) {
  fastNoise.SetFrequency(frequency);
  fastNoise.SetFractalOctaves(octaves);
}

Noise::~Noise() {}

double Noise::in2D(double x, double y) {
  return (1.0 + fastNoise.GetSimplexFractal(x, y)) / 2.0;
}
double Noise::in3D(double x, double y, double z) {
  return (1.0 + fastNoise.GetSimplexFractal(x, y, z)) / 2.0;
}
