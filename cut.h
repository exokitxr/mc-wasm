// #define CSGJS_HEADER_ONLY
// #include "csgjs.cpp"
// #include "earcut.hpp"
#include "TriangleMesh.hpp"
#include "meshoptimizer/src/meshoptimizer.h"

void cut(
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
);

void chunk(
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
);

void chunkOne(
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
);

void decimate(
  float *positions,
  unsigned int &numPositions,
  float *normals,
  unsigned int &numNormals,
  float *colors,
  unsigned int &numColors,
  float *uvs,
  unsigned int &numUvs,
  unsigned int *ids,
  unsigned int &numIds,
  float minTris,
  float quantization,
  float target_error,
  float aggressiveness,
  float base,
  int iterationOffset,
  unsigned int *faces,
  unsigned int &numFaces
);