#include <emscripten.h>
#include "util.h"
#include "compose.h"
// #include <iostream>

EMSCRIPTEN_KEEPALIVE void objectize(
  void *src, void *geometries, unsigned int *geometryIndex,
  unsigned int *blocks, unsigned int *blockTypes, int *dims, unsigned char *transparentVoxels, unsigned char *translucentVoxels, float *faceUvs, float *shift,
  float *positions, float *uvs, unsigned char *ssaos, float *frames, float *objectIndices, unsigned int *indices, unsigned int *objects
) {
  unsigned int positionIndex[NUM_CHUNKS_HEIGHT];
  unsigned int uvIndex[NUM_CHUNKS_HEIGHT];
  unsigned int ssaoIndex[NUM_CHUNKS_HEIGHT];
  unsigned int frameIndex[NUM_CHUNKS_HEIGHT];
  unsigned int objectIndexIndex[NUM_CHUNKS_HEIGHT];
  unsigned int indexIndex[NUM_CHUNKS_HEIGHT];
  unsigned int objectIndex[NUM_CHUNKS_HEIGHT];

  compose(
    src, geometries, geometryIndex,
    blocks, blockTypes, dims, transparentVoxels, translucentVoxels, faceUvs, shift,
    positions, uvs, ssaos, frames, objectIndices, indices, objects,
    positionIndex, uvIndex, ssaoIndex, frameIndex, objectIndexIndex, indexIndex, objectIndex
  );

  /* Local<Array> numPositions = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numUvs = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numSsaos = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numFrames = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numObjectIndices = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numIndices = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numObjects = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);

  for (unsigned int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    numPositions->Set(i, Number::New(args.GetIsolate(), positionIndex[i]));
    numUvs->Set(i, Number::New(args.GetIsolate(), uvIndex[i]));
    numSsaos->Set(i, Number::New(args.GetIsolate(), ssaoIndex[i]));
    numFrames->Set(i, Number::New(args.GetIsolate(), frameIndex[i]));
    numObjectIndices->Set(i, Number::New(args.GetIsolate(), objectIndexIndex[i]));
    numIndices->Set(i, Number::New(args.GetIsolate(), indexIndex[i]));
    numObjects->Set(i, Number::New(args.GetIsolate(), objectIndex[i]));
  }

  Local<Object> result = Object::New(args.GetIsolate());
  result->Set(positionsString, numPositions);
  result->Set(uvsString, numUvs);
  result->Set(ssaosString, numSsaos);
  result->Set(framesString, numFrames);
  result->Set(objectIndicesString, numObjectIndices);
  result->Set(indicesString, numIndices);
  result->Set(objectsString, numObjects);
  args.GetReturnValue().Set(result); */
}
