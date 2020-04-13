#include <emscripten.h>
#include <cstdlib>
// #include "util.h"
// #include "compose.h"
#include "march.h"
#include "uv.h"
#include "cut.h"
#include "cmprs.h"
// #include "light.h"
// #include "heightfield.h"
// #include "cull.h"
// #include "noiser.h"

extern "C" {

/* int main() {
  // std::cout << "main" << "\n";
  initUtil();
}

EMSCRIPTEN_KEEPALIVE Noiser *make_noiser(int seed) {
  return new Noiser(seed);
}

EMSCRIPTEN_KEEPALIVE void destroy_noiser(Noiser *noiser) {
  delete noiser;
}

EMSCRIPTEN_KEEPALIVE void noiser_apply(Noiser *noiser, int ox, int oz, unsigned char *biomes, unsigned char *temperature, unsigned char *humidity, bool fillBiomes, float *elevations, bool fillElevations, float *ethers, bool fillEther, float *water, float *lava, bool fillLiquid, float *newEther, unsigned int numNewEthers) {
  noiser->apply(ox, oz, biomes, temperature, humidity, fillBiomes, elevations, fillElevations, ethers, fillEther, water, lava, fillLiquid, newEther, numNewEthers);
}

EMSCRIPTEN_KEEPALIVE void noiser_fill(Noiser *noiser, int ox, int oz, unsigned char *biomes, float *elevations, float *ethers, float *water, float *lava, float *positions, unsigned int *indices, unsigned int *attributeRanges, unsigned int *indexRanges, float *staticHeightfield, float *colors, unsigned char *peeks) {
  noiser->fill(ox, oz, biomes, elevations, ethers, water, lava, positions, indices, attributeRanges, indexRanges, staticHeightfield, colors, peeks);
}

EMSCRIPTEN_KEEPALIVE void objectize(
  void *objectsSrc, void *vegetationsSrc, void *geometries, unsigned int *geometryIndex,
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
    objectsSrc, vegetationsSrc, geometries, geometryIndex,
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

EMSCRIPTEN_KEEPALIVE bool lght(
  int ox, int oz, int minX, int maxX, int minY, int maxY, int minZ, int maxZ, unsigned int relight,
  float **lavaArray, float **objectLightsArray, float **etherArray, unsigned int **blocksArray, unsigned char **lightsArray
) {
  return light(ox, oz, minX, maxX, minY, maxY, minZ, maxZ, (bool)relight, lavaArray, objectLightsArray, etherArray, blocksArray, lightsArray);
}

EMSCRIPTEN_KEEPALIVE void lghtmap(int ox, int oz, float *positions, unsigned int numPositions, float *staticHeightfield, unsigned char *lights, unsigned char *skyLightmaps, unsigned char *torchLightmaps) {
  lightmap(ox, oz, positions, numPositions, staticHeightfield, lights, skyLightmaps, torchLightmaps);
}

EMSCRIPTEN_KEEPALIVE void blockfield(unsigned int *blocks, unsigned char *blockfield) {
  genBlockfield(blocks, blockfield);
}

EMSCRIPTEN_KEEPALIVE void cllTerrain(float *hmdPosition, float *projectionMatrix, float *matrixWorldInverse, int frustumCulled, int *mapChunkMeshes, unsigned int numMapChunkMeshes, int *groups, int *groups2, unsigned int *groupIndices) {
  cullTerrain(hmdPosition, projectionMatrix, matrixWorldInverse, frustumCulled != 0, mapChunkMeshes, numMapChunkMeshes, groups, groups2, groupIndices[0], groupIndices[1]);
}

EMSCRIPTEN_KEEPALIVE unsigned int cllObjects(float *hmdPosition, float *projectionMatrix, float *matrixWorldInverse, int frustumCulled, int *mapChunkMeshes, unsigned int numMapChunkMeshes, int *groups) {
  return cullObjects(hmdPosition, projectionMatrix, matrixWorldInverse, frustumCulled != 0, mapChunkMeshes, numMapChunkMeshes, groups);
}

EMSCRIPTEN_KEEPALIVE void cllideBoxEther(int dims[3], float *potential, int shift[3], float *positionSpec, unsigned int *result) {
  bool collided;
  bool floored;
  bool ceiled;
  collideBoxEther(dims, potential, shift, positionSpec, collided, floored, ceiled);
  result[0] = (unsigned int)collided;
  result[1] = (unsigned int)floored;
  result[2] = (unsigned int)ceiled;
} */

/* EMSCRIPTEN_KEEPALIVE void doSmoothedPotentials(int *chunkCoords, unsigned int numChunkCoords, float *colorTargetCoordBuf, int colorTargetSize, float voxelSize, float *potentialsBuffer) {
  smoothedPotentials(chunkCoords, numChunkCoords, colorTargetCoordBuf, colorTargetSize, voxelSize, potentialsBuffer);
} */

EMSCRIPTEN_KEEPALIVE void doMarchingCubes(int dims[3], float *potential, uint8_t *brush, float shift[3], float scale[3], float *positions, float *colors, unsigned int *faces, unsigned int *positionIndex, unsigned int *colorIndex, unsigned int *faceIndex) {
  marchingCubes(dims, potential, brush, shift, scale, positions, colors, faces, *positionIndex, *colorIndex, *faceIndex);
}

EMSCRIPTEN_KEEPALIVE void doCollide(float *positions, unsigned int numPositions, float origin[3], float direction[3], float *collision, unsigned int *collisionIndex) {
  collide(positions, numPositions, origin, direction, collision, collisionIndex);
}

/* EMSCRIPTEN_KEEPALIVE void doComputeGeometry(int *chunkCoords, unsigned int numChunkCoords, float *colorTargetCoordBuf, int colorTargetSize, float voxelSize, float marchCubesTexSize, float marchCubesTexSquares, float marchCubesTexTriangleSize, float *potentialsBuffer, float *positions, float *barycentrics, float *uvs, float *uvs2, unsigned int *positionIndex, unsigned int *barycentricIndex, unsigned int *uvIndex, unsigned int *uvIndex2) {
  computeGeometry(chunkCoords, numChunkCoords, colorTargetCoordBuf, colorTargetSize, voxelSize, marchCubesTexSize, marchCubesTexSquares, marchCubesTexTriangleSize, potentialsBuffer, positions, barycentrics, uvs, uvs2, positionIndex, barycentricIndex, uvIndex, uvIndex2);
} */

EMSCRIPTEN_KEEPALIVE void doUvParameterize(float *positions, unsigned int numPositions, float *normals, unsigned int numNormals, unsigned int *faces, unsigned int numFaces, float *outPositions, unsigned int *numOutPositions, float *outNormals, unsigned int *numOutNormals, unsigned int *outFaces, float *uvs, unsigned int *numUvs) {
  uvParameterize(positions, numPositions, normals, numNormals, faces, numFaces, outPositions, *numOutPositions, outNormals, *numOutNormals, outFaces, uvs, *numUvs);
}

EMSCRIPTEN_KEEPALIVE void doCut(
  float *positions,
  unsigned int numPositions,
  unsigned int *faces,
  unsigned int numFaces,
  float *position,
  float *quaternion,
  float *scale,
  float *outPositions,
  unsigned int *numOutPositions,
  unsigned int *outFaces,
  unsigned int *numOutFaces
) {
  cut(
    positions,
    numPositions,
    faces,
    numFaces,
    position,
    quaternion,
    scale,
    outPositions,
    numOutPositions,
    outFaces,
    numOutFaces
  );
}

EMSCRIPTEN_KEEPALIVE void doCompress(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  uint8_t *outData,
  unsigned int *outSize
) {
  compress(
    positions,
    numPositions,
    normals,
    numNormals,
    colors,
    numColors,
    outData,
    outSize
  );
}

EMSCRIPTEN_KEEPALIVE void doDecompress(
  uint8_t *data,
  unsigned int size,
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors
) {
  decompress(
    data,
    size,
    positions,
    numPositions,
    normals,
    numNormals,
    colors,
    numColors
  );
}

EMSCRIPTEN_KEEPALIVE void doChunk(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  float *uvs,
  unsigned int numUvs,
  unsigned int *ids,
  unsigned int numIds,
  unsigned int *faces,
  unsigned int numFaces,
  float *mins,
  float *maxs,
  float *scale,
  float **outPositions,
  unsigned int *numOutPositions,
  float **outNormals,
  unsigned int *numOutNormals,
  float **outColors,
  unsigned int *numOutColors,
  float **outUvs,
  unsigned int *numOutUvs,
  unsigned int **outIds,
  unsigned int *numOutIds,
  unsigned int **outFaces,
  unsigned int *numOutFaces
) {
  chunk(
    positions,
    numPositions,
    normals,
    numNormals,
    colors,
    numColors,
    uvs,
    numUvs,
    ids,
    numIds,
    faces,
    numFaces,
    mins,
    maxs,
    scale,
    outPositions,
    numOutPositions,
    outNormals,
    numOutNormals,
    outColors,
    numOutColors,
    outUvs,
    numOutUvs,
    outIds,
    numOutIds,
    outFaces,
    numOutFaces
  );
}

EMSCRIPTEN_KEEPALIVE void doChunkOne(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  float *uvs,
  unsigned int numUvs,
  unsigned int *ids,
  unsigned int numIds,
  unsigned int *faces,
  unsigned int numFaces,
  float *mins,
  float *maxs,
  float *outP,
  unsigned int *numOutP,
  float *outN,
  unsigned int *numOutN,
  float *outC,
  unsigned int *numOutC,
  float *outU,
  unsigned int *numOutU,
  unsigned int *outX,
  unsigned int *numOutX,
  unsigned int *outI,
  unsigned int *numOutI
) {
  chunkOne(
    positions,
    numPositions,
    normals,
    numNormals,
    colors,
    numColors,
    uvs,
    numUvs,
    ids,
    numIds,
    faces,
    numFaces,
    mins,
    maxs,
    outP,
    numOutP,
    outN,
    numOutN,
    outC,
    numOutC,
    outU,
    numOutU,
    outX,
    numOutX,
    outI,
    numOutI
  );
}

EMSCRIPTEN_KEEPALIVE void doDecimate(
  float *positions,
  unsigned int *numPositions,
  float *normals,
  unsigned int *numNormals,
  float *colors,
  unsigned int *numColors,
  float *uvs,
  unsigned int *numUvs,
  float minTris,
  float aggressiveness,
  float base,
  int iterationOffset,
  unsigned int *faces,
  unsigned int *numFaces
) {
  decimate(
    positions,
    *numPositions,
    normals,
    *numNormals,
    colors,
    *numColors,
    uvs,
    *numUvs,
    minTris,
    aggressiveness,
    base,
    iterationOffset,
    faces,
    *numFaces
  );
}

EMSCRIPTEN_KEEPALIVE void *doMalloc(size_t size) {
  return malloc(size);
}

EMSCRIPTEN_KEEPALIVE void doFree(void *ptr) {
  free(ptr);
}

}
