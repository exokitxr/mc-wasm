#ifndef UTIL_H
#define UTIL_H

const int NUM_CELLS = 16;
const int OVERSCAN = 1;
const int NUM_CELLS_OVERSCAN = NUM_CELLS + OVERSCAN;
const int NUM_CELLS_HEIGHT = 128;
const int NUM_CHUNKS_HEIGHT = NUM_CELLS_HEIGHT / NUM_CELLS;
const int NUM_CELLS_OVERSCAN_Y = NUM_CELLS_HEIGHT + OVERSCAN;
const int HEIGHTFIELD_DEPTH = 8;
const int NUM_SLOTS = 64 * 64;
const int BLOCK_BUFFER_SIZE = 16 * 128 * 16 * 4;

int getCoordOverscanIndex(int x, int z);
int getEtherIndex(int x, int y, int z);
int getBlockIndex(int x, int y, int z);
int getLightsArrayIndex(int x, int z);
int getLightsIndex(int x, int y, int z);
int getTopHeightfieldIndex(int x, int z);
int getStaticHeightfieldIndex(int x, int z);

#endif
