#include "light.h"
#include "util.h"
#include <cmath>
#include <vector>
#include <queue>
// #include <iostream>

class LightSource {
  public:

    int x;
    int y;
    int z;
    char v;

    LightSource(int x, int y, int z, char v) : x(x), y(y), z(z), v(v) {}
};

inline bool isOccluded(int ox, int oz, int x, int y, int z, float **etherArray, unsigned int **blocksArray) {
  const int lax = (x - (ox - 1) * NUM_CELLS) >> 4;
  const int laz = (z - (oz - 1) * NUM_CELLS) >> 4;
  const int arrayIndex = getLightsArrayIndex(lax, laz);

  const int lx = x - (x & 0xFFFFFFF0);
  const int lz = z - (z & 0xFFFFFFF0);

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
    const int lax = (x - (ox - 1) * NUM_CELLS) >> 4;
    const int laz = (z - (oz - 1) * NUM_CELLS) >> 4;
    const int lightsArrayIndex = getLightsArrayIndex(lax, laz);
    unsigned char *lights = lightsArray[lightsArrayIndex];

    const int lx = x - (x & 0xFFFFFFF0);
    const int ly = y;
    const int lz = z - (z & 0xFFFFFFF0);
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
            lightSource.x + dx, lightSource.y + dy, lightSource.z + dz, lightSource.v - (char)(std::abs((float)dx) + std::abs((float)dy) + std::abs((float)dz)), false,
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
              (int)objectLights[offset + 0],
              (int)objectLights[offset + 1],
              (int)objectLights[offset + 2],
              (char)objectLights[offset + 3]
            )
          );
        }
      }
    }
  }
}

inline void setLight(int ox, int oz, int x, int y, int z, unsigned char v, unsigned char **lightsArray) {
  const int lax = (x - (ox - 1) * NUM_CELLS) >> 4;
  const int laz = (z - (oz - 1) * NUM_CELLS) >> 4;
  const int lightsArrayIndex = getLightsArrayIndex(lax, laz);
  unsigned char *lights = lightsArray[lightsArrayIndex];

  const int lx = x - (x & 0xFFFFFFF0);
  const int ly = y;
  const int lz = z - (z & 0xFFFFFFF0);
  const int lightsIndex = getLightsIndex(lx, ly, lz);
  lights[lightsIndex] = v;
}

inline unsigned char getLight(int ox, int oz, int x, int y, int z, unsigned char **lightsArray) {
  const int lax = (x - (ox - 1) * NUM_CELLS) >> 4;
  const int laz = (z - (oz - 1) * NUM_CELLS) >> 4;
  const int lightsArrayIndex = getLightsArrayIndex(lax, laz);
  unsigned char *lights = lightsArray[lightsArrayIndex];

  const int lx = x - (x & 0xFFFFFFF0);
  const int ly = y;
  const int lz = z - (z & 0xFFFFFFF0);
  const int lightsIndex = getLightsIndex(lx, ly, lz);
  return lights[lightsIndex];
}

void light(int ox, int oz, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, bool relight, float **lavaArray, float **objectLightsArray, float **etherArray, unsigned int **blocksArray, unsigned char **lightsArray) {
  std::vector<LightSource> lightSources;
  getLightSources(ox, oz, lavaArray, objectLightsArray, lightSources);
  if (relight) {
    int x, y, z;
    // top
    y = maxY;
    if (y <= NUM_CELLS_HEIGHT) {
      for (int z = minZ; z < maxZ; z++) {
        for (int x = minX; x < maxX; x++) {
          lightSources.push_back(LightSource(x, y, z, getLight(ox, oz, x, y, z, lightsArray)));
        }
      }
    }
    // bottom
    y = minY;
    if (y >= 0) {
      for (int z = minZ; z < maxZ; z++) {
        for (int x = minX; x < maxX; x++) {
          lightSources.push_back(LightSource(x, y, z, getLight(ox, oz, x, y, z, lightsArray)));
        }
      }
    }
    // left
    x = minX;
    if (x >= (ox - 1) * NUM_CELLS) {
      for (int z = minZ; z < maxZ; z++) {
        for (int y = minY; y <= maxY; y++) {
          lightSources.push_back(LightSource(x, y, z, getLight(ox, oz, x, y, z, lightsArray)));
        }
      }
    }
    // right
    x = maxX - 1;
    if (x < (ox + 2) * NUM_CELLS) {
      for (int z = minZ; z < maxZ; z++) {
        for (int y = minY; y <= maxY; y++) {
          lightSources.push_back(LightSource(x, y, z, getLight(ox, oz, x, y, z, lightsArray)));
        }
      }
    }
    // back
    z = minZ;
    if (z >= (oz - 1) * NUM_CELLS) {
      for (int x = minX; x < maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
          lightSources.push_back(LightSource(x, y, z, getLight(ox, oz, x, y, z, lightsArray)));
        }
      }
    }
    // front
    z = maxZ - 1;
    if (z < (oz + 2) * NUM_CELLS) {
      for (int x = minX; x < maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
          lightSources.push_back(LightSource(x, y, z, getLight(ox, oz, x, y, z, lightsArray)));
        }
      }
    }
  }

  for (int z = minZ; z < maxZ; z++) {
    for (int x = minX; x < maxX; x++) {
      for (int y = minY; y <= maxY; y++) {
        setLight(ox, oz, x, y, z, 0, lightsArray);
      }
    }
  }

  for (const LightSource &lightSource : lightSources) {
    fillLight(ox, oz, lightSource.x, lightSource.y, lightSource.z, lightSource.v, minX, maxX, minY, maxY, minZ, maxZ, etherArray, blocksArray, lightsArray);
  }

  // merge edges and corners
  for (unsigned int doz = 0; doz <= 2; doz++) {
    for (unsigned int dox = 0; dox <= 2; dox++) {
      unsigned char *centerLights = lightsArray[getLightsArrayIndex(dox, doz)];

      if (dox + 1 <= 2) {
        unsigned char *eastLights = lightsArray[getLightsArrayIndex(dox + 1, doz)];
        for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
          for (int y = 0; y < (NUM_CELLS_HEIGHT + 1); y++) {
            centerLights[getLightsIndex(NUM_CELLS, y, z)] = eastLights[getLightsIndex(0, y, z)];
          }
        }
      }
      if (doz + 1 <= 2) {
        unsigned char *southLights = lightsArray[getLightsArrayIndex(dox, doz + 1)];
        for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
          for (int y = 0; y < (NUM_CELLS_HEIGHT + 1); y++) {
            centerLights[getLightsIndex(x, y, NUM_CELLS)] = southLights[getLightsIndex(x, y, 0)];
          }
        }
      }
      if (dox + 1 <= 2 && doz + 1 <=2) {
        unsigned char *southeastLights = lightsArray[getLightsArrayIndex(dox + 1, doz + 1)];
        for (int y = 0; y < (NUM_CELLS_HEIGHT + 1); y++) {
          centerLights[getLightsIndex(NUM_CELLS, y, NUM_CELLS)] = southeastLights[getLightsIndex(0, y, 0)];
        }
      }
    }
  }
}

inline unsigned char renderSkyVoxel(int x, int y, int z, float *staticHeightfield) {
  return (unsigned char)(
    std::min<float>(std::max<float>(
      (y - (staticHeightfield[getStaticHeightfieldIndex(x, z)] - 8.0)) / 8.0
    , 0), 1) * 255.0
  );
}
inline unsigned char renderTorchVoxel(int x, int y, int z, unsigned char *lights) {
  return (unsigned char)((std::min<int>(lights[getLightsIndex(x, y, z)], 15) * 255) / 15);
}

void lightmap(int ox, int oz, float *positions, unsigned int numPositions, float *staticHeightfield, unsigned char *lights, unsigned char *skyLightmaps, unsigned char *torchLightmaps) {
  const int dox = ox * NUM_CELLS;
  const int doz = oz * NUM_CELLS;

  for (unsigned int i = 0; i < numPositions / 3; i++) {
    unsigned int baseIndex = i * 3;
    const int x = std::min<int>(std::max<int>((int)positions[baseIndex + 0] - dox, 0), NUM_CELLS);
    const int y = std::min<int>(std::max<int>((int)positions[baseIndex + 1], 0), NUM_CELLS_HEIGHT + 1);
    const int z = std::min<int>(std::max<int>((int)positions[baseIndex + 2] - doz, 0), NUM_CELLS);
    skyLightmaps[i] = renderSkyVoxel(
      x,
      y,
      z,
      staticHeightfield
    );
    torchLightmaps[i] = renderTorchVoxel(
      x,
      y,
      z,
      lights
    );
  }
}
