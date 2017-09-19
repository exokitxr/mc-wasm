#ifndef NOISER_H
#define NOISER_H

#include "noise.h"
#include "cachedNoise.h"
#include "hash.h"
#include <random>
#include <unordered_map>

class Noiser {
  public:
    std::mt19937 rng;
    Noise elevationNoise1;
    Noise elevationNoise2;
    Noise elevationNoise3;
    CachedNoise wormNoise;
    CachedNoise oceanNoise;
    CachedNoise riverNoise;
    CachedNoise temperatureNoise;
    CachedNoise humidityNoise;

    std::unordered_map<std::pair<int, int>, unsigned char> biomeCache;
    std::unordered_map<std::tuple<unsigned char, int, int>, float> biomeHeightCache;
    std::unordered_map<std::pair<int, int>, float> elevationCache;

    explicit Noiser(int seed);

    unsigned char getBiome(int x, int z);
    float getBiomeHeight(unsigned char b, int x, int z);
    float getElevation(int x, int z);
    double getTemperature(double x, double z);
    void fillBiomes(int ox, int oz, unsigned char *biomes);
    void fillElevations(int ox, int oz, float *elevations);
    void fillEther(float *elevations, float *ether);
    void fillLiquid(int ox, int oz, float *ether, float *elevations, float *water, float *lava);
    void applyEther(float *newEther, unsigned int numNewEthers, float *ether);
    void makeGeometries(int ox, int oy, float *ether, float *water, float *lava, float *positions, unsigned int *indices, unsigned int *attributeRanges, unsigned int *indexRanges);
    void fill(int ox, int oz, unsigned char *biomes, bool fillBiomes, float *elevations, bool fillElevations, float *ethers, bool fillEther, float *water, float *lava, bool fillLiquid, float *newEther, unsigned int numNewEthers, float *positions, unsigned int *indices, unsigned int *attributeRanges, unsigned int *indexRanges, float *heightfield, float *staticHeightfield, float *colors, unsigned char *peeks);
    void postProcessGeometry(int ox, int oz, unsigned int *attributeRanges, float *positions, float *colors, unsigned char *biomes);
};

#endif
