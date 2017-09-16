#include "util.h"
#include "vector.h"
#include <cmath>
// #include <iostream>

void handleHeightfieldPoint(const Vec &point, float *heightfield, float *staticHeightfield) {
  const int x = (int)std::floor(point.x);
  const float y = point.y;
  const int z = (int)std::floor(point.z);

  for (int layer = 0; layer < HEIGHTFIELD_DEPTH; layer++) {
    const int heightfieldXYBaseIndex = getTopHeightfieldIndex(x, z);
// std::cout << "got point " << point.x << " : " << point.z << " : " << x << " : " << y << " : " << z << " : " << heightfieldXYBaseIndex << " : " << layer << "\n";
    const float oldY = heightfield[heightfieldXYBaseIndex + layer];
    if (y > oldY) {
      if (layer == 0 || (y - oldY) >= 5) { // ignore non-surface heights with small height difference
        for (int k = HEIGHTFIELD_DEPTH - 1; k > layer; k--) {
          heightfield[heightfieldXYBaseIndex + k] = heightfield[heightfieldXYBaseIndex + k - 1];
        }
        heightfield[heightfieldXYBaseIndex + layer] = y;
      }
      break;
    } else if (y == oldY) {
      break;
    }
  }

  const unsigned int staticheightfieldIndex = getStaticHeightfieldIndex(x, z);
  if (y > staticHeightfield[staticheightfieldIndex]) {
    staticHeightfield[staticheightfieldIndex] = y;
  }
}

void genHeightfield(float *positions, unsigned int numPositions, float *heightfield, float *staticHeightfield) {
  for (unsigned int i = 0; i < NUM_CELLS_OVERSCAN * NUM_CELLS_OVERSCAN * HEIGHTFIELD_DEPTH; i++) {
    heightfield[i] = -1024.0f;
  }
  for (unsigned int i = 0; i < NUM_CELLS_OVERSCAN * NUM_CELLS_OVERSCAN; i++) {
    staticHeightfield[i] = -1024.0f;
  }

  for (unsigned int baseIndex = 0; baseIndex < numPositions; baseIndex += 9) {
    Tri localTriangle(
      Vec(positions[baseIndex + 0], positions[baseIndex + 1], positions[baseIndex + 2]),
      Vec(positions[baseIndex + 3], positions[baseIndex + 4], positions[baseIndex + 5]),
      Vec(positions[baseIndex + 6], positions[baseIndex + 7], positions[baseIndex + 8])
    );
    if (localTriangle.normal().y > 0) {
      handleHeightfieldPoint(localTriangle.a, heightfield, staticHeightfield);
      handleHeightfieldPoint(localTriangle.b, heightfield, staticHeightfield);
      handleHeightfieldPoint(localTriangle.c, heightfield, staticHeightfield);
    }
  }
}
