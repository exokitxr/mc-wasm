#include "light.h"
#include "util.h"
#include "v8-strings.h"
#include <node.h>
#include <cmath>
#include <vector>
#include <queue>
// #include <iostream>

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::Array;
using v8::ArrayBuffer;

class LightSource {
  public:

    int x;
    int y;
    int z;
    char v;

    LightSource(int x, int y, int z, char v) : x(x), y(y), z(z), v(v) {}
};

inline bool isOccluded(int ox, int oz, int x, int y, int z, float **etherArray, unsigned int **blocksArray) {
  const int lx = x - (int)std::floor((float)x / (float)NUM_CELLS) * NUM_CELLS;
  const int lz = z - (int)std::floor((float)z / (float)NUM_CELLS) * NUM_CELLS;

  const int lax = (int)std::floor(((float)x - ((float)ox - 1.0) * (float)NUM_CELLS) / (float)NUM_CELLS);
  const int laz = (int)std::floor(((float)z - ((float)oz - 1.0) * (float)NUM_CELLS) / (float)NUM_CELLS);
  const int arrayIndex = getLightsArrayIndex(lax, laz);

  float *ether = etherArray[arrayIndex];
  if (ether[getEtherIndex(lx, y, lz)] <= -1) {
    return true;
  }
  unsigned int *blocks = blocksArray[arrayIndex];
  if (blocks[getBlockIndex(lx, y, lz)] != 0) {
    return true;
  }
  return false;
}

inline void tryQueue(int ox, int oz, int x, int y, int z, char v, bool origin, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, float **etherArray, unsigned int **blocksArray, std::queue<LightSource> &queue, unsigned char **lightsArray) {
  if (x >= minX && x < maxX && y >= minY && y <= maxY && z >= minZ && z < maxZ && v > 0) {
    const int lax = (int)std::floor(((float)x - ((float)ox - 1.0) * (float)NUM_CELLS) / (float)NUM_CELLS);
    const int laz = (int)std::floor(((float)z - ((float)oz - 1.0) * (float)NUM_CELLS) / (float)NUM_CELLS);
    const int lightsArrayIndex = getLightsArrayIndex(lax, laz);
    unsigned char *lights = lightsArray[lightsArrayIndex];

    const int lx = x - (int)std::floor((float)x / (float)NUM_CELLS) * NUM_CELLS;
    const int ly = y;
    const int lz = z - (int)std::floor((float)z / (float)NUM_CELLS) * NUM_CELLS;
    const int lightsIndex = getLightsIndex(lx, ly, lz);
    if (lights[lightsIndex] < v) {
      lights[lightsIndex] = v;

      if (origin || !isOccluded(ox, oz, x, y, z, etherArray, blocksArray)) {
        queue.push(LightSource(x, y, z, v));
      }
    }
  }
}

inline void fillLight(int ox, int oz, int x, int y, int z, char v, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, float **etherArray, unsigned int **blocksArray, unsigned char **lightsArray) {
  std::queue<LightSource> queue;

  tryQueue(ox, oz, x, y, z, v, true, minX, maxX, minY, maxY, minZ, maxZ, etherArray, blocksArray, queue, lightsArray);

  while (queue.size() > 0) {
    const LightSource lightSource = queue.front();
    queue.pop();
    for (int dz = -1; dz <= 1; dz++) {
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          tryQueue(ox, oz,
            lightSource.x + dx, lightSource.y + dy, lightSource.z + dz, lightSource.v - (char)(std::abs(dx) + std::abs(dy) + std::abs(dz)), false,
            minX, maxX, minY, maxY, minZ, maxZ,
            etherArray, blocksArray, queue, lightsArray
          );
        }
      }
    }
  }
};

void getLightSources(int ox, int oz, float **lavaArray, float **objectLightsArray, std::vector<LightSource> &lightSources) {
  for (int dz = -1; dz <= 1; dz++) {
    for (int dx = -1; dx <= 1; dx++) {
      const int aox = ox + dx;
      const int aoz = oz + dz;

      const int arrayIndex = getLightsArrayIndex(dx + 1, dz + 1);
      float *lava = lavaArray[arrayIndex];
      unsigned int index = 0;
      for (int y = 0; y <= NUM_CELLS_HEIGHT; y++) {
        for (int z = 0; z <= NUM_CELLS; z++) {
          for (int x = 0; x <= NUM_CELLS; x++) {
            if (lava[index++] < 0) {
              lightSources.push_back(
                LightSource(
                  x + aox * NUM_CELLS,
                  y,
                  z + aoz * NUM_CELLS,
                  15
                )
              );
            }
          }
        }
      }

      float *objectLights = objectLightsArray[arrayIndex];
      for (int i = 0; i < NUM_SLOTS; i++) {
        const int offset = i * 4;
        if (objectLights[offset + 3] > 0) {
          lightSources.push_back(
            LightSource(
              (int)std::floor(objectLights[offset + 0]),
              (int)std::floor(objectLights[offset + 1]),
              (int)std::floor(objectLights[offset + 2]),
              (char)std::floor(objectLights[offset + 3])
            )
          );
        }
      }
    }
  }
}

void light(int ox, int oz, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, float **lavaArray, float **objectLightsArray, float **etherArray, unsigned int **blocksArray, unsigned char **lightsArray) {
  for (int z = minZ; z < maxZ; z++) {
    for (int x = minX; x < maxX; x++) {
      for (int y = minY; y <= maxY; y++) {
        const int lax = (int)std::floor(((float)x - ((float)ox - 1.0) * (float)NUM_CELLS) / (float)NUM_CELLS);
        const int laz = (int)std::floor(((float)z - ((float)oz - 1.0) * (float)NUM_CELLS) / (float)NUM_CELLS);
        const int lightsArrayIndex = getLightsArrayIndex(lax, laz);
        unsigned char *lights = lightsArray[lightsArrayIndex];

        const int lx = x - (int)std::floor((float)x / (float)NUM_CELLS) * NUM_CELLS;
        const int ly = y;
        const int lz = z - (int)std::floor((float)z / (float)NUM_CELLS) * NUM_CELLS;
        const int lightsIndex = getLightsIndex(lx, ly, lz);
        lights[lightsIndex] = 0;
      }
    }
  }
 
  std::vector<LightSource> lightSources;
  getLightSources(ox, oz, lavaArray, objectLightsArray, lightSources);
  for (auto const &lightSource : lightSources) {
    fillLight(ox, oz, lightSource.x, lightSource.y, lightSource.z, lightSource.v, minX, maxX, minY, maxY, minZ, maxZ, etherArray, blocksArray, lightsArray);
  }

  // merge edges and corner into center lights
  unsigned char *centerLights = lightsArray[getLightsArrayIndex(1, 1)];
  unsigned char *eastLights = lightsArray[getLightsArrayIndex(2, 1)];
  unsigned char *southLights = lightsArray[getLightsArrayIndex(1, 2)];
  unsigned char *southeastLights = lightsArray[getLightsArrayIndex(2, 2)];
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int y = 0; y < (NUM_CELLS_HEIGHT + 1); y++) {
      centerLights[getLightsIndex(NUM_CELLS, y, z)] = eastLights[getLightsIndex(0, y, z)];
    }
  }
  for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
    for (int y = 0; y < (NUM_CELLS_HEIGHT + 1); y++) {
      centerLights[getLightsIndex(x, y, NUM_CELLS)] = southLights[getLightsIndex(x, y, 0)];
    }
  }
  for (int y = 0; y < (NUM_CELLS_HEIGHT + 1); y++) {
    centerLights[getLightsIndex(NUM_CELLS, y, NUM_CELLS)] = southeastLights[getLightsIndex(0, y, 0)];
  }
}
