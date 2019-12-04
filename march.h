#ifndef MARCH_H
#define MARCH_H

void marchingCubes(int dims[3], float *potential, float shift[3], float marchCubesTexSize, float marchCubesTexSquares, float marchCubesTexTriangleSize, float *positions, float *barycentrics, float *uvs, float *uvs2, unsigned int &positionIndex, unsigned int &barycentricIndex, unsigned int &uvIndex, unsigned int &uvIndex2);
void collide(float *positions, unsigned int *indices, unsigned int numPositions, unsigned int numIndices, float origin[3], float direction[3], float *positionSpec);

#endif
