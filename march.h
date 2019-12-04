#ifndef MARCH_H
#define MARCH_H

void marchingCubes(int dims[3], float *potential, float shift[3], int indexOffset, float *positions, float *coords, unsigned int *faces, unsigned int &positionIndex, unsigned int &coordIndex, unsigned int &faceIndex);
void collide(float *positions, unsigned int *indices, unsigned int numPositions, unsigned int numIndices, float origin[3], float direction[3], float *positionSpec);

#endif
