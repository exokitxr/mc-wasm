#include "util.h"

int getCoordOverscanIndex(int x, int z) {
  return x + z * NUM_CELLS_OVERSCAN;
}
int getEtherIndex(int x, int y, int z) {
  return x + (z * NUM_CELLS_OVERSCAN) + (y * NUM_CELLS_OVERSCAN * NUM_CELLS_OVERSCAN);
}
int getBlockIndex(int x, int y, int z) {
  const int ay = y / 16;
  y = y - ay * 16;
  return (ay * (BLOCK_BUFFER_SIZE / 4 / (128 / 16))) + x + y * 16 + z * 16 * 16;
}
int getLightsArrayIndex(int x, int z) {
  return x + z * 3;
}
int getLightsIndex(int x, int y, int z) {
  return x + y * NUM_CELLS_OVERSCAN + z * NUM_CELLS_OVERSCAN * (NUM_CELLS_HEIGHT + 1);
}
int getTopHeightfieldIndex(int x, int z) {
  return (x + (z * NUM_CELLS_OVERSCAN)) * HEIGHTFIELD_DEPTH;
}
int getStaticHeightfieldIndex(int x, int z) {
  return x + (z * NUM_CELLS_OVERSCAN);
}
