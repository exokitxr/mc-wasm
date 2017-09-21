#ifndef CULL_H
#define CULL_H

unsigned int cullTerrain(float *hmdPosition, float *projectionMatrix, float *matrixWorldInverse, int *mapChunkMeshes, unsigned int numMapChunkMeshes, int *groups);

#endif
