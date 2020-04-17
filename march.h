#ifndef MARCH_H
#define MARCH_H

#include "vector.h"
#include <cstdlib>
#include <limits>
#include <array>
#include <deque>
#include <map>
#include <set>

class ChunkKey {
public:
  int x;
  int y;
  int z;
  int lod;
};
class ChunkVector {
public:
  int x;
  int y;
  int z;
};
class ChunkVoxels {
public:
  ChunkVoxels();
  ChunkVoxels(int voxelWidth, float nvalue);
  std::vector<float> voxels;
};

void smoothedPotentials(int *chunkCoords, unsigned int numChunkCoords, float *colorTargetCoordBuf, int colorTargetSize, float voxelSize, float *potentialsBuffer);
void marchingCubes(int dims[3], float *potential, uint8_t *brush, float shift[3], float scale[3], float *positions, float *colors, unsigned int *faces, unsigned int &positionIndex, unsigned int &colorIndex, unsigned int &faceIndex);
void decimateMarch(int dims[3], float shift[3], float size[3], float *positions, unsigned int *faces, unsigned int &positionIndex, unsigned int &faceIndex);
void marchPotentials(int x, int y, int z, int lod, int *dims, float *shift, float *size, float *positions, float *barycentrics, unsigned int &positionIndex, unsigned int &barycentricIndex);
void computeGeometry(int *chunkCoords, unsigned int numChunkCoords, float *colorTargetCoordBuf, int colorTargetSize, float voxelSize, float marchCubesTexSize, float marchCubesTexSquares, float marchCubesTexTriangleSize, float *potentialsBuffer, float *positions, float *barycentrics, float *uvs, float *uvs2, unsigned int *positionIndex, unsigned int *barycentricIndex, unsigned int *uvIndex, unsigned int *uvIndex2);
void pushChunkTexture(int x, int y, int z, int lod, float *depthTextures, int voxelWidth, float voxelSize, float voxelResolution, float value, float nvalue);
void collide(float *positions, unsigned int numPositions, float origin[3], float direction[3], float *collision, unsigned int *collisionIndex);

#endif
