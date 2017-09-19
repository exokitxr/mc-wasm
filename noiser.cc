#include "noiser.h"
#include "util.h"
#include "biomes.h"
#include "noise.h"
#include "cachedNoise.h"
#include "march.h"
#include "heightfield.h"
#include "flod.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>

Noiser::Noiser(int seed) :
  rng(seed),
  elevationNoise1(rng(), 2, 1),
  elevationNoise2(rng(), 2, 1),
  elevationNoise3(rng(), 2, 1),
  wormNoise(rng(), 0.002, 4),
  oceanNoise(rng(), 0.002, 4),
  riverNoise(rng(), 0.002, 4),
  temperatureNoise(rng(), 0.002, 4),
  humidityNoise(rng(), 0.002, 4)
{}

unsigned char Noiser::getBiome(int x, int z) {
  const std::pair<int, int> key(x, z);
  std::unordered_map<std::pair<int, int>, unsigned char>::iterator entryIter = biomeCache.find(key);

  if (entryIter != biomeCache.end()) {
    return entryIter->second;
  } else {
    unsigned char &biome = biomeCache[key];

    biome = 0xFF;
    if (oceanNoise.in2D(x + 1000, z + 1000) < (90.0 / 255.0)) {
      biome = (unsigned char)BIOME::biOcean;
    }
    if (biome == 0xFF) {
      const double n = riverNoise.in2D(x + 1000, z + 1000);
      const double range = 0.04;
      if (n > 0.5 - range && n < 0.5 + range) {
        biome = (unsigned char)BIOME::biRiver;
      }
    }
    if (temperatureNoise.in2D(x + 1000, z + 1000) < ((4.0 * 16.0) / 255.0)) {
      if (biome == (unsigned char)BIOME::biOcean) {
        biome = (unsigned char)BIOME::biFrozenOcean;
      } else if (biome == (unsigned char)BIOME::biRiver) {
        biome = (unsigned char)BIOME::biFrozenRiver;
      }
    }
    if (biome == 0xFF) {
      const int t = (int)std::floor(temperatureNoise.in2D(x + 1000, z + 1000) * 16.0);
      const int h = (int)std::floor(humidityNoise.in2D(x + 1000, z + 1000) * 16.0);
      biome = (unsigned char)BIOMES_TEMPERATURE_HUMIDITY[t + 16 * h];
    }

    return biome;
  }
}

float Noiser::getBiomeHeight(unsigned char b, int x, int z) {
  const std::tuple<unsigned char, int, int> key(b, x, z);
  std::unordered_map<std::tuple<unsigned char, int, int>, float>::iterator entryIter = biomeHeightCache.find(key);

  if (entryIter != biomeHeightCache.end()) {
    return entryIter->second;
  } else {
    float &biomeHeight = biomeHeightCache[key];

    const Biome &biome = BIOMES[b];
    biomeHeight = biome.baseHeight +
      elevationNoise1.in2D(x * biome.amps[0][0], z * biome.amps[0][0]) * biome.amps[0][1] +
      elevationNoise2.in2D(x * biome.amps[1][0], z * biome.amps[1][0]) * biome.amps[1][1] +
      elevationNoise3.in2D(x * biome.amps[2][0], z * biome.amps[2][0]) * biome.amps[2][1];

    return biomeHeight;
  }
}

float Noiser::getElevation(int x, int z) {
  const std::pair<int, int> key(x, z);
  std::unordered_map<std::pair<int, int>, float>::iterator entryIter = elevationCache.find(key);

  if (entryIter != elevationCache.end()) {
    return entryIter->second;
  } else {
    float &elevation = elevationCache[key];

    std::unordered_map<unsigned char, unsigned int> biomeCounts;
    for (int dz = -8; dz <= 8; dz++) {
      for (int dx = -8; dx <= 8; dx++) {
        biomeCounts[getBiome(x + dx, z + dz)]++;
      }
    }

    float elevationSum = 0;
    for (auto const &iter : biomeCounts) {
      elevationSum += iter.second * getBiomeHeight(iter.first, x, z);
    }
    elevation = elevationSum / ((8 * 2 + 1) * (8 * 2 + 1));

    return elevation;
  }
}

double Noiser::getTemperature(double x, double z) {
  return temperatureNoise.in2D(x, z);
}

void Noiser::fillBiomes(int ox, int oz, unsigned char *biomes) {
  unsigned int index = 0;
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      biomes[index++] = getBiome((ox * NUM_CELLS) + x, (oz * NUM_CELLS) + z);
    }
  }
}

void Noiser::fillElevations(int ox, int oz, float *elevations) {
  unsigned int index = 0;
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      elevations[index++] = getElevation((ox * NUM_CELLS) + x, (oz * NUM_CELLS) + z);
    }
  }
}

void Noiser::fillEther(float *elevations, float *ether) {
  unsigned int index = 0;
  for (int y = 0; y < NUM_CELLS_OVERSCAN_Y; y++) {
    for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
      for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
        const float elevation = elevations[x + z * NUM_CELLS_OVERSCAN];
        ether[index++] = std::min<float>(std::max<float>((float)y - elevation, -1.0), 1.0);
      }
    }
  }
}

inline void setLiquid(int ox, int oz, int x, int y, int z, float *liquid) {
  x -= ox * NUM_CELLS;
  z -= oz * NUM_CELLS;

  const float factor = sqrt(3.0) * 0.8;

  for (int dz = -1; dz <= 1; dz++) {
    const int az = z + dz;
    if (az >= 0 && az <= NUM_CELLS) {
      for (int dx = -1; dx <= 1; dx++) {
        const int ax = x + dx;
        if (ax >= 0 && ax <= NUM_CELLS) {
          for (int dy = -1; dy <= 1; dy++) {
            const int ay = y + dy;
            if (ay >= 0 && ay < (NUM_CELLS_HEIGHT + 1)) {
              const int index = getEtherIndex(ax, ay, az);
              liquid[index] = std::min<float>(-1.0 * (1.0 - (sqrt((float)dx*(float)dx + (float)dy*(float)dy + (float)dz*(float)dz) / factor)), liquid[index]);
            }
          }
        }
      }
    }
  }
}

void Noiser::fillLiquid(int ox, int oz, float *ether, float *elevations, float *water, float *lava) {
  for (unsigned int i = 0; i < (NUM_CELLS + 1) * (NUM_CELLS_HEIGHT + 1) * (NUM_CELLS + 1); i++) {
    water[i] = 1.0;
    lava[i] = 1.0;
  }

  // water
  unsigned int index = 0;
  for (int z = 0; z <= NUM_CELLS; z++) {
    for (int x = 0; x <= NUM_CELLS; x++) {
      const float elevation = elevations[index++];
      for (int y = elevation; y < 64; y++) {
        if (y < 64 && y >= elevation) {
          const int index = getEtherIndex(x, y, z);
          water[index] = ether[index] * -1;
        }
      }
    }
  }

  // lava
  index = 0;
  for (int doz = -1; doz <= 1; doz++) {
    for (int dox = -1; dox <= 1; dox++) {
      for (int z = 0; z <= NUM_CELLS; z++) {
        for (int x = 0; x <= NUM_CELLS; x++) {
          const int ax = ((ox + dox) * NUM_CELLS) + x;
          const int az = ((oz + doz) * NUM_CELLS) + z;
          const float elevation = getElevation(ax, az);

          if (elevation >= 80 && getTemperature(ax + 1000, az + 1000) < 0.235) {
            setLiquid(ox, oz, ax, (int)std::floor(elevation + 1.0), az, lava);
          }
        }
      }
    }
  }
}

void Noiser::applyEther(float *newEther, unsigned int numNewEthers, float *ether) {
  unsigned int baseIndex = 0;
  for (unsigned int i = 0; i < numNewEthers; i++) {
    const float x = newEther[baseIndex + 0];
    const float y = newEther[baseIndex + 1];
    const float z = newEther[baseIndex + 2];
    const float v = newEther[baseIndex + 3];
    for (int dz = -HOLE_SIZE; dz <= HOLE_SIZE; dz++) {
      const int az = z + dz;
      if (az >= 0 && az < (NUM_CELLS + 1)) {
        for (int dx = -HOLE_SIZE; dx <= HOLE_SIZE; dx++) {
          const int ax = x + dx;
          if (ax >= 0 && ax < (NUM_CELLS + 1)) {
            for (int dy = -HOLE_SIZE; dy <= HOLE_SIZE; dy++) {
              const int ay = y + dy;
              if (ay >= 0 && ay < (NUM_CELLS_HEIGHT + 1)) {
                ether[getEtherIndex(ax, ay, az)] += v * std::max<float>((float)HOLE_SIZE - std::sqrt((float)dx * (float)dx + (float)dy * (float)dy + (float)dz * (float)dz), 0) / std::sqrt((float)HOLE_SIZE * (float)HOLE_SIZE * 3.0);
              }
            }
          }
        }
      }
    }
    baseIndex += 4;
  }
}

void Noiser::makeGeometries(int ox, int oy, float *ether, float *water, float *lava, float *positionsBuffer, unsigned int *indicesBuffer, unsigned int *attributeRanges, unsigned int *indexRanges) {
  int attributeIndex = 0;
  int indexIndex = 0;

  // land
  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    unsigned int positionIndex;
    unsigned int faceIndex;

    int dims[3] = {
      NUM_CELLS + 1,
      NUM_CELLS + 1,
      NUM_CELLS + 1
    };
    float *potential = ether;
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    int indexOffset = attributeIndex / 3;
    float *positions = (float *)((char *)positionsBuffer + attributeIndex * 4);
    unsigned int *faces = (unsigned int *)((char *)indicesBuffer + indexIndex * 4);

    marchingCubes(dims, potential, shift, indexOffset, positions, faces, positionIndex, faceIndex);

    attributeRanges[i * 6 + 0] = attributeIndex;
    attributeRanges[i * 6 + 1] = positionIndex;
    indexRanges[i * 6 + 0] = indexIndex;
    indexRanges[i * 6 + 1] = faceIndex,

    attributeIndex += positionIndex;
    indexIndex += faceIndex;
  }

  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
{
    unsigned int positionIndex;
    unsigned int faceIndex;

    // water
    int dims[3] = {
      NUM_CELLS + 1,
      NUM_CELLS + 1,
      NUM_CELLS + 1
    };
    float *potential = water;
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    int indexOffset = attributeIndex / 3;
    float *positions = (float *)((char *)positionsBuffer + attributeIndex * 4);
    unsigned int *faces = (unsigned int *)((char *)indicesBuffer + indexIndex * 4);

    marchingCubes(dims, potential, shift, indexOffset, positions, faces, positionIndex, faceIndex);

    attributeRanges[i * 6 + 2] = attributeIndex;
    attributeRanges[i * 6 + 3] = positionIndex;
    indexRanges[i * 6 + 2] = indexIndex;
    indexRanges[i * 6 + 3] = faceIndex;

    attributeIndex += positionIndex;
    indexIndex += faceIndex;
}
{
    unsigned int positionIndex;
    unsigned int faceIndex;

    // lava
    int dims[3] = {
      NUM_CELLS + 1,
      NUM_CELLS + 1,
      NUM_CELLS + 1
    };
    float *potential = lava;
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    int indexOffset = attributeIndex / 3;
    float *positions = (float *)((char *)positionsBuffer + attributeIndex * 4);
    unsigned int *faces = (unsigned int *)((char *)indicesBuffer + indexIndex * 4);

    marchingCubes(dims, potential, shift, indexOffset, positions, faces, positionIndex, faceIndex);

    attributeRanges[i * 6 + 4] = attributeIndex;
    attributeRanges[i * 6 + 5] = positionIndex;
    indexRanges[i * 6 + 4] = indexIndex;
    indexRanges[i * 6 + 5] = faceIndex;

    attributeIndex += positionIndex;
    indexIndex += faceIndex;
}
  }
}


void Noiser::fill(int ox, int oz, unsigned char *biomes, bool fillBiomes, float *elevations, bool fillElevations, float *ethers, bool fillEther, float *water, float *lava, bool fillLiquid, float *newEther, unsigned int numNewEthers, float *positions, unsigned int *indices, unsigned int *attributeRanges, unsigned int *indexRanges, float *heightfield, float *staticHeightfield, float *colors, unsigned char *peeks) {
  if (fillBiomes) {
    this->fillBiomes(ox, oz, biomes);
  }
  if (fillElevations) {
    this->fillElevations(ox, oz, elevations);
  }
  if (fillEther) {
    this->fillEther(elevations, ethers);
  }
  if (fillLiquid) {
    this->fillLiquid(ox, oz, ethers, elevations, water, lava);
  }
  if (numNewEthers > 0) {
    this->applyEther(newEther, numNewEthers, ethers);
  }

  this->makeGeometries(ox, oz, ethers, water, lava, positions, indices, attributeRanges, indexRanges);

  unsigned int numIndices = indexRanges[5 * 6 + 4] + indexRanges[5 * 6 + 5];
  genHeightfield(positions, indices, numIndices, heightfield, staticHeightfield);

  this->postProcessGeometry(ox, oz, attributeRanges, positions, colors, biomes);

  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    flod(ethers, shift, peeks + i * 16);
  }
}

inline void postProcessGeometryRange(int ox, int oz, unsigned int start, unsigned int count, float *positions, float *colors, const std::function<void(const float,const float,const float,const float,const float,float &,float &,float &)>& getColor) {
  float *geometryPositions = positions + start;
  float *geometryColors = colors + start;

  unsigned int baseIndex = 0;
  for (unsigned int i = 0; i < count / 3; i++) {
    const float x = geometryPositions[baseIndex + 0];
    const float y = geometryPositions[baseIndex + 1];
    const float z = geometryPositions[baseIndex + 2];

    const float ax = x + (ox * NUM_CELLS);
    const float ay = y;
    const float az = z + (oz * NUM_CELLS);

    geometryPositions[baseIndex + 0] = ax;
    geometryPositions[baseIndex + 2] = az;

    getColor(ox, oz, ax, ay, az, geometryColors[baseIndex + 0], geometryColors[baseIndex + 1], geometryColors[baseIndex + 2]);

    baseIndex += 3;
  }
};

void Noiser::postProcessGeometry(int ox, int oz, unsigned int *attributeRanges, float *positions, float *colors, unsigned char *biomes) {
  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    unsigned int landStart = attributeRanges[i * 6 + 0];
    unsigned int landCount = attributeRanges[i * 6 + 1];
    postProcessGeometryRange(ox, oz, landStart, landCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
      const Biome &biome = BIOMES[biomes[getCoordOverscanIndex((int)x - ox * NUM_CELLS, (int)z - oz * NUM_CELLS)]];
      const unsigned int color = biome.color;
      r = ((color >> (8 * 2)) & 0xFF) / 255.0;
      g = ((color >> (8 * 1)) & 0xFF) / 255.0;
      b = ((color >> (8 * 0)) & 0xFF) / 255.0;
    });

    unsigned int waterStart = attributeRanges[i * 6 + 2];
    unsigned int waterCount = attributeRanges[i * 6 + 3];
    postProcessGeometryRange(ox, oz, waterStart, waterCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
      r = 0.0;
      g = 0.0;
      b = 1.0;
    });

    unsigned int lavaStart = attributeRanges[i * 6 + 4];
    unsigned int lavaCount = attributeRanges[i * 6 + 5];
    postProcessGeometryRange(ox, oz, lavaStart, lavaCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
      r = 0.5;
      g = 0.0;
      b = 2.0;
    });
  }
}
