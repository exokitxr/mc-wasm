#include "compose.h"
#include "tssl.h"
#include <node.h>
#include <string.h>
#include <cmath>
#include <algorithm>
#include <memory>
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
const unsigned int BLOCK_BUFFER_SIZE = 16 * 128 * 16 * 4;
const unsigned int NUM_VOXELS_CHUNKS_HEIGHT = BLOCK_BUFFER_SIZE / 4 / NUM_CHUNKS_HEIGHT;
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

    positionIndex += this->numPositions;
    uvIndex += this->numUvs;
    ssaoIndex += this->numSsaos;
    frameIndex += this->numFrames;
    objectIndexIndex += this->numObjectIndices;
    indexIndex += this->numIndices;
    objectIndex += this->numObjects;
  }
};

void compose(
  void *src, void *geometries, Local<Object> &geometryIndex,
  unsigned int *blocks, Local<Object> &blockTypes, int dims[3], unsigned char *transparentVoxels, unsigned char *translucentVoxels, float *faceUvs, float *shift,
  float *positions, float *uvs, unsigned char *ssaos, float *frames, float *objectIndices, unsigned int *indices, unsigned int *objects,
  unsigned int *positionIndex, unsigned int *uvIndex, unsigned int *ssaoIndex, unsigned int *frameIndex, unsigned int *objectIndexIndex, unsigned int *indexIndex, unsigned int *objectIndex
) {

  std::vector<unsigned int> objectsArray[NUM_CHUNKS_HEIGHT];
  unsigned int offset = 0;
  for (unsigned int i = 0; i < OBJECT_SLOTS; i++) {
    const unsigned int n = *((unsigned int *)((char *)src + offset));
    offset += 4;

    if (n != 0) {
      float *positionBuffer = (float *)((char *)src + offset);
      const float y = positionBuffer[1];
      const int chunkIndex = (int)std::floor(std::min<float>(std::max<float>(y, 0), (float)(NUM_CELLS_HEIGHT - 1)) / (float)NUM_CELLS);
      objectsArray[chunkIndex].push_back(i);
    }

    offset += 4 * 11;
  }

  for (unsigned int chunkIndex = 0; chunkIndex < NUM_CHUNKS_HEIGHT; chunkIndex++) {
    if (chunkIndex == 0) {
      positionIndex[chunkIndex] = 0;
      uvIndex[chunkIndex] = 0;
      ssaoIndex[chunkIndex] = 0;
      frameIndex[chunkIndex] = 0;
      objectIndexIndex[chunkIndex] = 0;
      indexIndex[chunkIndex] = 0;
      objectIndex[chunkIndex] = 0;
    } else {
      positionIndex[chunkIndex] = positionIndex[chunkIndex - 1];
      uvIndex[chunkIndex] = uvIndex[chunkIndex - 1];
      ssaoIndex[chunkIndex] = ssaoIndex[chunkIndex - 1];
      frameIndex[chunkIndex] = frameIndex[chunkIndex - 1];
      objectIndexIndex[chunkIndex] = objectIndexIndex[chunkIndex - 1];
      indexIndex[chunkIndex] = indexIndex[chunkIndex - 1];
      objectIndex[chunkIndex] = objectIndex[chunkIndex - 1];
    }

    std::vector<unsigned int> &objectsVector = objectsArray[chunkIndex];
    for (const unsigned int &i : objectsVector) {
      unsigned int offset = i * 4 * 12;

      const unsigned int n = *((unsigned int *)((char *)src + offset));
      offset += 4;

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

      std::unique_ptr<Geometry> geometry(
        new Geometry(
          (char *)geometries + geometryIndex->Get(n)->Uint32Value(),
          i,
          positionIndex[chunkIndex],
          uvIndex[chunkIndex],
          ssaoIndex[chunkIndex],
          frameIndex[chunkIndex],
          objectIndexIndex[chunkIndex],
          indexIndex[chunkIndex],
          objectIndex[chunkIndex]
        )
      );
      geometry->applyRotation(rotation);
      geometry->applyTranslation(position);
      geometry->write(
        positions,
        uvs,
        ssaos,
        frames,
        objectIndices,
        indices,
        objects,
        positionIndex[chunkIndex],
        uvIndex[chunkIndex],
        ssaoIndex[chunkIndex],
        frameIndex[chunkIndex],
        objectIndexIndex[chunkIndex],
        indexIndex[chunkIndex],
        objectIndex[chunkIndex]
      );
    }

    unsigned int *voxels = blocks + (chunkIndex * NUM_VOXELS_CHUNKS_HEIGHT);
    shift[1] = chunkIndex * NUM_CELLS; // XXX clean this up
    const unsigned int numPositions = positionIndex[chunkIndex];
    tesselate(
      voxels,
      blockTypes,
      dims,
      transparentVoxels,
      translucentVoxels,
      faceUvs,
      shift,
      numPositions,
      positions + positionIndex[chunkIndex],
      uvs + uvIndex[chunkIndex],
      ssaos + ssaoIndex[chunkIndex],
      frames + frameIndex[chunkIndex],
      objectIndices + objectIndexIndex[chunkIndex],
      indices + indexIndex[chunkIndex],
      positionIndex[chunkIndex],
      uvIndex[chunkIndex],
      ssaoIndex[chunkIndex],
      frameIndex[chunkIndex],
      objectIndexIndex[chunkIndex],
      indexIndex[chunkIndex]
    );
  }
}
