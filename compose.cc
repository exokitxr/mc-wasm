#include "compose.h"
#include <node.h>
#include <string.h>
#include <cmath>
#include <algorithm>
#include "vector.h"
// #include <iostream>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Number;
using v8::Value;
using v8::Array;
using v8::ArrayBuffer;
using v8::Float32Array;
using v8::Uint32Array;
using v8::Uint8Array;

const unsigned int NUM_CELLS = 16;
const unsigned int NUM_CELLS_HEIGHT = 128;
const unsigned int NUM_CHUNKS_HEIGHT = NUM_CELLS_HEIGHT / NUM_CELLS;
const unsigned int OBJECT_SLOTS = 64 * 64;
const unsigned int GEOMETRY_BUFFER_SIZE = 100 * 1024;

unsigned int _align(unsigned int n, unsigned int alignment) {
  unsigned int alignDiff = n % alignment;
  if (alignDiff > 0) {
    n += alignment - alignDiff;
  }
  return n;
}

class Geometry {
  public:

  float positions[GEOMETRY_BUFFER_SIZE];
  float uvs[GEOMETRY_BUFFER_SIZE];
  unsigned char ssaos[GEOMETRY_BUFFER_SIZE];
  float frames[GEOMETRY_BUFFER_SIZE];
  float objectIndices[GEOMETRY_BUFFER_SIZE];
  unsigned int indices[GEOMETRY_BUFFER_SIZE];
  unsigned int objects[GEOMETRY_BUFFER_SIZE];
  unsigned int positionIndex;
  unsigned int uvIndex;
  unsigned int ssaoIndex;
  unsigned int frameIndex;
  unsigned int objectIndexIndex;
  unsigned int indexIndex;
  unsigned int objectIndex;
  unsigned int numPositions;
  unsigned int numUvs;
  unsigned int numSsaos;
  unsigned int numFrames;
  unsigned int numObjectIndices;
  unsigned int numIndices;
  unsigned int numObjects;

  Geometry(void *geometries, unsigned int i, unsigned int positionIndex, unsigned int uvIndex, unsigned int ssaoIndex, unsigned int frameIndex, unsigned int objectIndexIndex, unsigned int indexIndex, unsigned int objectIndex) :
    positionIndex(positionIndex),
    uvIndex(uvIndex),
    ssaoIndex(ssaoIndex),
    frameIndex(frameIndex),
    objectIndexIndex(objectIndexIndex),
    indexIndex(indexIndex),
    objectIndex(objectIndex)
  {
// std::cout << "read 1 " << i << ":" << positionIndex << "\n";

    unsigned int byteOffset = 0;

    unsigned int *headerBuffer = (unsigned int *)geometries;
    unsigned int index = 0;
    numPositions = headerBuffer[index++];
    numUvs = headerBuffer[index++];
    numSsaos = headerBuffer[index++];
    numFrames = headerBuffer[index++];
    numIndices = headerBuffer[index++];
    byteOffset += 5 * 4;

    float *positionsBuffer = (float *)((char *)geometries + byteOffset);
    memcpy(positions, positionsBuffer, numPositions * 4);
    byteOffset += 4 * numPositions;

    float *uvsBuffer = (float *)((char *)geometries + byteOffset);
    for (unsigned int j = 0; j < numUvs / 2; j++) {
      unsigned int baseIndex = j * 2;
      uvs[baseIndex + 0] = uvsBuffer[baseIndex + 0];
      uvs[baseIndex + 1] = 1.0f - uvsBuffer[baseIndex + 1];
    }
    byteOffset += 4 * numUvs;

    unsigned char *ssaosBuffer = (unsigned char *)((char *)geometries + byteOffset);
    memcpy(ssaos, ssaosBuffer, numSsaos);
    byteOffset += 1 * numSsaos;
    byteOffset = _align(byteOffset, 4);

    float *framesBuffer = (float *)((char *)geometries + byteOffset);
    memcpy(frames, framesBuffer, numFrames * 4);
    byteOffset += 4 * numFrames;

    numObjectIndices = numPositions / 3;
    for (unsigned int j = 0; j < numObjectIndices; j++) {
      objectIndices[j] = i;
    }

    unsigned int *indicesBuffer = (unsigned int *)((char *)geometries + byteOffset);
    for (unsigned int j = 0; j < numIndices; j++) {
      indices[j] = indicesBuffer[j] + positionIndex / 3;
    }
    byteOffset += 4 * numIndices;

    numObjects = 7;
    float *boundingBoxBuffer = (float *)((char *)geometries + byteOffset);
    objects[0] = i;
    memcpy(objects + 1, boundingBoxBuffer, 6 * 4);
    byteOffset += 4 * 7;

// std::cout << "geometry " << numPositions << ":" << numUvs << ":" << numSsaos << ":" << numFrames << ":" << numObjectIndices << ":" << numIndices << ":" << numObjects  << "\n";

    /* positionIndex += numPositions;
    uvIndex += numUvs;
    ssaoIndex += numSsaos;
    frameIndex += numFrames;
    objectIndexIndex += numObjectIndices;
    indexIndex += numIndices;
    objectIndex += numObjects; */
  }

  void applyTranslation(const Vec &v) {
    for (unsigned int i = 0; i < numPositions / 3; i++) {
      unsigned int baseIndex = i * 3;
      positions[baseIndex + 0] += v.x;
      positions[baseIndex + 1] += v.y;
      positions[baseIndex + 2] += v.z;
    }
  }

  void applyRotation(const Quat &q) {
    const Vec u(q.x, q.y, q.z);

    for (unsigned int i = 0; i < numPositions / 3; i++) {
      unsigned int baseIndex = i * 3;
      const Vec v(positions[baseIndex + 0], positions[baseIndex + 1], positions[baseIndex + 2]);

      const Vec u(q.x, q.y, q.z);
      const float s = q.w;

      const Vec result = (u * (2.0f * (u * v))) +
        (v * (s*s - (u * u))) + 
        ((u ^ v) * (2.0f * s));

      positions[baseIndex + 0] = result.x;
      positions[baseIndex + 1] = result.y;
      positions[baseIndex + 2] = result.z;
    }
  }

  void write(float *positions, float *uvs, unsigned char *ssaos, float *frames, float *objectIndices, unsigned int *indices, unsigned int *objects, unsigned int &positionIndex, unsigned int &uvIndex, unsigned int &ssaoIndex, unsigned int &frameIndex, unsigned int &objectIndexIndex, unsigned int &indexIndex, unsigned int &objectIndex) const {
    memcpy(positions + this->positionIndex, this->positions, this->numPositions * 4);
    memcpy(uvs + this->uvIndex, this->uvs, this->numUvs * 4);
    memcpy(ssaos + this->ssaoIndex, this->ssaos, this->numSsaos * 1);
    memcpy(frames + this->frameIndex, this->frames, this->numFrames * 4);
    memcpy(objectIndices + this->objectIndexIndex, this->objectIndices, this->numObjectIndices * 4);
    memcpy(indices + this->indexIndex, this->indices, this->numIndices * 4);
    memcpy(objects + this->objectIndex, this->objects, this->numObjects * 4);

// std::cout << "write 1 " << positionIndex << ":" << this->positionIndex << "\n";
    positionIndex += this->numPositions;
// std::cout << "write 2 " << positionIndex << ":" << this->positionIndex << "\n";
    uvIndex += this->numUvs;
    ssaoIndex += this->numSsaos;
    frameIndex += this->numFrames;
    objectIndexIndex += this->numObjectIndices;
    indexIndex += this->numIndices;
    objectIndex += this->numObjects;
  }
};

void compose(void *src, void *geometries, Local<Object> &geometryIndex, float **positions, float **uvs, unsigned char **ssaos, float **frames, float **objectIndices, unsigned int **indices, unsigned int **objects, unsigned int *positionIndex, unsigned int *uvIndex, unsigned int *ssaoIndex, unsigned int *frameIndex, unsigned int *objectIndexIndex, unsigned int *indexIndex, unsigned int *objectIndex) {
  for (unsigned int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    positionIndex[i] = 0;
    uvIndex[i] = 0;
    ssaoIndex[i] = 0;
    frameIndex[i] = 0;
    objectIndexIndex[i] = 0;
    indexIndex[i] = 0;
    objectIndex[i] = 0;
  }

  unsigned int offset = 0;
  for (unsigned int i = 0; i < OBJECT_SLOTS; i++) {
    const unsigned int n = *((unsigned int *)((char *)src + offset));
    offset += 4;

    if (n != 0) {
      float *positionBuffer = (float *)((char *)src + offset);
      const Vec position(
        positionBuffer[0],
        positionBuffer[1],
        positionBuffer[2]
      );
      offset += 4 * 3;

      float *rotationBuffer = (float *)((char *)src + offset);
      const Quat rotation(
        rotationBuffer[0],
        rotationBuffer[1],
        rotationBuffer[2],
        rotationBuffer[3]
      );
      offset += 4 * 4;
      offset += 4 * 3; // scale
      offset += 4 * 1; // value

// std::cout << "geometry from index " << n << " : " << geometryIndex->Get(n)->Uint32Value() << "\n";
/* if (geometryIndex->Get(n)->Uint32Value() == 0) {
  std::cout << "failed to look up geometry " << n << "\n";
} */
      const int chunkIndex = (int)std::floor(std::min<float>(std::max<float>(position.y, 0), (float)(NUM_CELLS_HEIGHT - 1)) / (float)NUM_CELLS);
      Geometry geometry((char *)geometries + geometryIndex->Get(n)->Uint32Value(), i, positionIndex[chunkIndex], uvIndex[chunkIndex], ssaoIndex[chunkIndex], frameIndex[chunkIndex], objectIndexIndex[chunkIndex], indexIndex[chunkIndex], objectIndex[chunkIndex]);
      geometry.applyRotation(rotation);
      geometry.applyTranslation(position);
      geometry.write(positions[chunkIndex], uvs[chunkIndex], ssaos[chunkIndex], frames[chunkIndex], objectIndices[chunkIndex], indices[chunkIndex], objects[chunkIndex], positionIndex[chunkIndex], uvIndex[chunkIndex], ssaoIndex[chunkIndex], frameIndex[chunkIndex], objectIndexIndex[chunkIndex], indexIndex[chunkIndex], objectIndex[chunkIndex]);

      // std::cout << "write 3 " << positionIndex[i] << "\n";
    } else {
      offset += 4 * 11;
    }
  }
}
