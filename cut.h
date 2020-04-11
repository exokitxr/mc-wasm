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
  unsigned int *numOutUvs
);

void decimate(
  float *positions,
  unsigned int numPositions,
  float factor,
  unsigned int *outFaces,
  unsigned int *numOutFaces
);