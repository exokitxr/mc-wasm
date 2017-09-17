#ifndef COMPOSE_H
#define COMPOSE_H

#include <node.h>

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

void compose(
  void *src, void *geometries, Local<Object> &geometryIndex,
  unsigned int *blocks, Local<Object> &blockTypes, int dims[3], unsigned char *transparentVoxels, unsigned char *translucentVoxels, float *faceUvs, float *shift,
  float *positions, float *uvs, unsigned char *ssaos, float *frames, float *objectIndices, unsigned int *indices, unsigned int *objects,
  unsigned int *positionIndex, unsigned int *uvIndex, unsigned int *ssaoIndex, unsigned int *frameIndex, unsigned int *objectIndexIndex, unsigned int *indexIndex, unsigned int *objectIndex
);

#endif
