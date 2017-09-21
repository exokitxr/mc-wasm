#include <emscripten.h>
#include "util.h"
#include "compose.h"
#include "light.h"
#include "noiser.h"
// #include <iostream>

extern "C" {

EMSCRIPTEN_KEEPALIVE Noiser *make_noiser(int seed) {
  return new Noiser(seed);
}

EMSCRIPTEN_KEEPALIVE void destroy_noiser(Noiser *noiser) {
  delete noiser;
}

EMSCRIPTEN_KEEPALIVE void noiser_fill(Noiser *noiser, int ox, int oz, unsigned char *biomes, bool fillBiomes, float *elevations, bool fillElevations, float *ethers, bool fillEther, float *water, float *lava, bool fillLiquid, float *newEther, unsigned int numNewEthers, float *positions, unsigned int *indices, unsigned int *attributeRanges, unsigned int *indexRanges, float *heightfield, float *staticHeightfield, float *colors, unsigned char *peeks) {
  noiser->fill(ox, oz, biomes, fillBiomes, elevations, fillElevations, ethers, fillEther, water, lava, fillLiquid, newEther, numNewEthers, positions, indices, attributeRanges, indexRanges, heightfield, staticHeightfield, colors, peeks);
}

EMSCRIPTEN_KEEPALIVE void objectize(
  void *src, void *geometries, unsigned int *geometryIndex,
  unsigned int *blocks, unsigned int *blockTypes, int *dims, unsigned char *transparentVoxels, unsigned char *translucentVoxels, float *faceUvs, float *shift,
  float *positions, float *uvs, unsigned char *ssaos, float *frames, float *objectIndices, unsigned int *indices, unsigned int *objects,
  unsigned int *result
) {
  unsigned int positionIndex[NUM_CHUNKS_HEIGHT];
  unsigned int uvIndex[NUM_CHUNKS_HEIGHT];
  unsigned int ssaoIndex[NUM_CHUNKS_HEIGHT];
  unsigned int frameIndex[NUM_CHUNKS_HEIGHT];
  unsigned int objectIndexIndex[NUM_CHUNKS_HEIGHT];
  unsigned int indexIndex[NUM_CHUNKS_HEIGHT];
  unsigned int objectIndex[NUM_CHUNKS_HEIGHT];

  compose(
    src, geometries, geometryIndex,
    blocks, blockTypes, dims, transparentVoxels, translucentVoxels, faceUvs, shift,
    positions, uvs, ssaos, frames, objectIndices, indices, objects,
    positionIndex, uvIndex, ssaoIndex, frameIndex, objectIndexIndex, indexIndex, objectIndex
  );

  for (unsigned int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    result[NUM_CHUNKS_HEIGHT * 0 + i] = positionIndex[i];
    result[NUM_CHUNKS_HEIGHT * 1 + i] = uvIndex[i];
    result[NUM_CHUNKS_HEIGHT * 2 + i] = ssaoIndex[i];
    result[NUM_CHUNKS_HEIGHT * 3 + i] = frameIndex[i];
    result[NUM_CHUNKS_HEIGHT * 4 + i] = objectIndexIndex[i];
    result[NUM_CHUNKS_HEIGHT * 5 + i] = indexIndex[i];
    result[NUM_CHUNKS_HEIGHT * 6 + i] = objectIndex[i];
  }
}

EMSCRIPTEN_KEEPALIVE void lght(
  int ox, int oz, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, unsigned int relight,
  float **lavaArray, float **objectLightsArray, float **etherArray, unsigned int **blocksArray, unsigned char **lightsArray
) {
  light(ox, oz, minX, maxX, minY, maxY, minZ, maxZ, (bool)relight, lavaArray, objectLightsArray, etherArray, blocksArray, lightsArray);
}

EMSCRIPTEN_KEEPALIVE void lghtmap(int ox, int oz, float *positions, unsigned int numPositions, float *staticHeightfield, unsigned char *lights, unsigned char *skyLightmaps, unsigned char *torchLightmaps) {
  lightmap(ox, oz, positions, numPositions, staticHeightfield, lights, skyLightmaps, torchLightmaps);
}


class Lol {
  EMSCRIPTEN_KEEPALIVE int zol(int *arg) {
    // std::cout << "got arg " << (void *)this << " : " << (void *)arg << "\n";
    return 7;
  }
};

}
