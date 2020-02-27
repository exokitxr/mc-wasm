#include <stddef.h>
#include <iostream>
#include "xatlas.h"

void uvParameterize(float *positions, unsigned int numPositions, float *normals, unsigned int numNormals, unsigned int *faces, unsigned int numFaces, float *outPositions, unsigned int &numOutPositions, float *outNormals, unsigned int &numOutNormals, unsigned int *outFaces, float *uvs, unsigned int &numUvs);