#include <stdlib.h>
#include <stdint.h>

void compress(
  float *positions,
  unsigned int numPositions,
  float *normals,
  unsigned int numNormals,
  float *colors,
  unsigned int numColors,
  uint8_t *outData,
  unsigned int *outSize
);