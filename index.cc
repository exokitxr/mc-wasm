#include <node.h>
#include "tssl.h"
#include "compose.h"
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

void Compose(const FunctionCallbackInfo<Value>& args) {
  Local<String> srcString = String::NewFromUtf8(args.GetIsolate(), "src");
  Local<String> geometriesString = String::NewFromUtf8(args.GetIsolate(), "geometries");
  Local<String> geometryIndexString = String::NewFromUtf8(args.GetIsolate(), "geometryIndex");
  Local<String> positionsString = String::NewFromUtf8(args.GetIsolate(), "positions");
  Local<String> uvsString = String::NewFromUtf8(args.GetIsolate(), "uvs");
  Local<String> ssaosString = String::NewFromUtf8(args.GetIsolate(), "ssaos");
  Local<String> framesString = String::NewFromUtf8(args.GetIsolate(), "frames");
  Local<String> objectIndicesString = String::NewFromUtf8(args.GetIsolate(), "objectIndices");
  Local<String> indicesString = String::NewFromUtf8(args.GetIsolate(), "indices");
  Local<String> objectsString = String::NewFromUtf8(args.GetIsolate(), "objects");
  Local<String> bufferString = String::NewFromUtf8(args.GetIsolate(), "buffer");
  Local<String> byteOffsetString = String::NewFromUtf8(args.GetIsolate(), "byteOffset");

  Local<Object> opts = args[0]->ToObject();

  Local<ArrayBuffer> srcBuffer = Local<ArrayBuffer>::Cast(opts->Get(srcString)->ToObject()->Get(bufferString));
  unsigned int srcByteOffset = opts->Get(srcString)->ToObject()->Get(byteOffsetString)->Uint32Value();
  void *src = (void *)((char *)srcBuffer->GetContents().Data() + srcByteOffset);

  Local<ArrayBuffer> geometriesBuffer = Local<ArrayBuffer>::Cast(opts->Get(geometriesString)->ToObject()->Get(bufferString));
  unsigned int geometriesByteOffset = opts->Get(geometriesString)->ToObject()->Get(byteOffsetString)->Uint32Value();
  void *geometries = (void *)((char *)geometriesBuffer->GetContents().Data() + geometriesByteOffset);

  Local<Object> geometryIndex = opts->Get(geometryIndexString)->ToObject();

  float *positions[NUM_CHUNKS_HEIGHT];
  float *uvs[NUM_CHUNKS_HEIGHT];
  unsigned char *ssaos[NUM_CHUNKS_HEIGHT];
  float *frames[NUM_CHUNKS_HEIGHT];
  float *objectIndices[NUM_CHUNKS_HEIGHT];
  unsigned int *indices[NUM_CHUNKS_HEIGHT];
  unsigned int *objects[NUM_CHUNKS_HEIGHT];
  unsigned int positionIndex[NUM_CHUNKS_HEIGHT];
  unsigned int uvIndex[NUM_CHUNKS_HEIGHT];
  unsigned int ssaoIndex[NUM_CHUNKS_HEIGHT];
  unsigned int frameIndex[NUM_CHUNKS_HEIGHT];
  unsigned int objectIndexIndex[NUM_CHUNKS_HEIGHT];
  unsigned int indexIndex[NUM_CHUNKS_HEIGHT];
  unsigned int objectIndex[NUM_CHUNKS_HEIGHT];

  for (unsigned int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    Local<ArrayBuffer> positionsBuffer = Local<ArrayBuffer>::Cast(opts->Get(positionsString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int positionsByteOffset = opts->Get(positionsString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    positions[i] = (float *)((char *)positionsBuffer->GetContents().Data() + positionsByteOffset);

    Local<ArrayBuffer> uvsBuffer = Local<ArrayBuffer>::Cast(opts->Get(uvsString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int uvsByteOffset = opts->Get(uvsString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    uvs[i] = (float *)((char *)uvsBuffer->GetContents().Data() + uvsByteOffset);

    Local<ArrayBuffer> ssaosBuffer = Local<ArrayBuffer>::Cast(opts->Get(ssaosString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int ssaosByteOffset = opts->Get(ssaosString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    ssaos[i] = (unsigned char *)((char *)ssaosBuffer->GetContents().Data() + ssaosByteOffset);

    Local<ArrayBuffer> framesBuffer = Local<ArrayBuffer>::Cast(opts->Get(framesString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int framesByteOffset = opts->Get(framesString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    frames[i] = (float *)((char *)framesBuffer->GetContents().Data() + framesByteOffset);

    Local<ArrayBuffer> objectIndicesBuffer = Local<ArrayBuffer>::Cast(opts->Get(objectIndicesString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int objectIndicesByteOffset = opts->Get(objectIndicesString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    objectIndices[i] = (float *)((char *)objectIndicesBuffer->GetContents().Data() + objectIndicesByteOffset);

    Local<ArrayBuffer> indicesBuffer = Local<ArrayBuffer>::Cast(opts->Get(indicesString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int indicesByteOffset = opts->Get(indicesString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    indices[i] = (unsigned int *)((char *)indicesBuffer->GetContents().Data() + indicesByteOffset);

    Local<ArrayBuffer> objectsBuffer = Local<ArrayBuffer>::Cast(opts->Get(objectsString)->ToObject()->Get(i)->ToObject()->Get(bufferString));
    unsigned int objectsByteOffset = opts->Get(objectsString)->ToObject()->Get(byteOffsetString)->Uint32Value();
    objects[i] = (unsigned int *)((char *)objectsBuffer->GetContents().Data() + objectsByteOffset);
  }

  compose(src, geometries, geometryIndex, positions, uvs, ssaos, frames, objectIndices, indices, objects, positionIndex, uvIndex, ssaoIndex, frameIndex, objectIndexIndex, indexIndex, objectIndex);

  Local<Array> numPositions = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numUvs = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numSsaos = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numFrames = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numObjectIndices = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numIndices = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);
  Local<Array> numObjects = Array::New(args.GetIsolate(), NUM_CHUNKS_HEIGHT);

  for (unsigned int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
// std::cout << "write 4 " << positionIndex[i] << "\n";
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
  args.GetReturnValue().Set(result);
}
 
void Tssl(const FunctionCallbackInfo<Value>& args) {
  unsigned int positionIndex;
  unsigned int uvIndex;
  unsigned int ssaoIndex;

  Local<String> bufferString = String::NewFromUtf8(args.GetIsolate(), "buffer");
  Local<String> byteOffsetString = String::NewFromUtf8(args.GetIsolate(), "byteOffset");
  Local<String> positionsString = String::NewFromUtf8(args.GetIsolate(), "positions");
  Local<String> uvsString = String::NewFromUtf8(args.GetIsolate(), "uvs");
  Local<String> ssaosString = String::NewFromUtf8(args.GetIsolate(), "ssaos");

  Local<ArrayBuffer> voxelsBuffer = Local<ArrayBuffer>::Cast(args[0]->ToObject()->Get(bufferString));
  unsigned int voxelsByteOffset = args[0]->ToObject()->Get(byteOffsetString)->Uint32Value();
  Local<Object> blockTypes = args[1]->ToObject();
  Local<Array> dimsArg = Local<Array>::Cast(args[2]);
  Local<ArrayBuffer> transparentVoxelsBuffer = Local<ArrayBuffer>::Cast(args[3]->ToObject()->Get(bufferString));
  unsigned int transparentVoxelsByteOffset = args[3]->ToObject()->Get(byteOffsetString)->Uint32Value();
  Local<ArrayBuffer> translucentVoxelsBuffer = Local<ArrayBuffer>::Cast(args[4]->ToObject()->Get(bufferString));
  unsigned int translucentVoxelsByteOffset = args[4]->ToObject()->Get(byteOffsetString)->Uint32Value();
  Local<ArrayBuffer> faceUvsBuffer = Local<ArrayBuffer>::Cast(args[5]->ToObject()->Get(bufferString));
  unsigned int faceUvsByteOffset = args[5]->ToObject()->Get(byteOffsetString)->Uint32Value();
  Local<Array> shiftArg = Local<Array>::Cast(args[6]);
  Local<ArrayBuffer> positionsBuffer = Local<ArrayBuffer>::Cast(args[7]->ToObject()->Get(bufferString));
  unsigned int positionsByteOffset = args[7]->ToObject()->Get(byteOffsetString)->Uint32Value();
  Local<ArrayBuffer> uvsBuffer = Local<ArrayBuffer>::Cast(args[8]->ToObject()->Get(bufferString));
  unsigned int uvsByteOffset = args[8]->ToObject()->Get(byteOffsetString)->Uint32Value();
  Local<ArrayBuffer> ssaosBuffer = Local<ArrayBuffer>::Cast(args[9]->ToObject()->Get(bufferString));
  unsigned int ssaosByteOffset = args[9]->ToObject()->Get(byteOffsetString)->Uint32Value();

  int dims[3] = {
    dimsArg->Get(0)->Int32Value(),
    dimsArg->Get(1)->Int32Value(),
    dimsArg->Get(2)->Int32Value()
  };
  float shift[3] = {
    (float)shiftArg->Get(0)->NumberValue(),
    (float)shiftArg->Get(1)->NumberValue(),
    (float)shiftArg->Get(2)->NumberValue()
  };

  unsigned int *voxels = (unsigned int *)((char *)voxelsBuffer->GetContents().Data() + voxelsByteOffset);
  unsigned char *transparentVoxels = (unsigned char *)((char *)transparentVoxelsBuffer->GetContents().Data() + transparentVoxelsByteOffset);
  unsigned char *translucentVoxels = (unsigned char *)((char *)translucentVoxelsBuffer->GetContents().Data() + translucentVoxelsByteOffset);
  float *faceUvs = (float *)((char *)faceUvsBuffer->GetContents().Data() + faceUvsByteOffset);
  float *positions = (float *)((char *)positionsBuffer->GetContents().Data() + positionsByteOffset);
  float *uvs = (float *)((char *)uvsBuffer->GetContents().Data() + uvsByteOffset);
  unsigned char *ssaos = (unsigned char *)((char *)ssaosBuffer->GetContents().Data() + ssaosByteOffset);

  // std::cout << "got init " << potential[99] << ":" << potential[100] << ":" << shift[0] << ":" << shift[1] << ":" << shift[2] << "\n";

  tesselate(voxels, blockTypes, dims, transparentVoxels, translucentVoxels, faceUvs, shift, positions, uvs, ssaos, positionIndex, uvIndex, ssaoIndex);

  Local<Object> result = Object::New(args.GetIsolate());
  result->Set(positionsString, Float32Array::New(positionsBuffer, positionsByteOffset, positionIndex));
  result->Set(uvsString, Float32Array::New(uvsBuffer, uvsByteOffset, uvIndex));
  result->Set(ssaosString, Uint8Array::New(ssaosBuffer, ssaosByteOffset, ssaoIndex));
  args.GetReturnValue().Set(result);
}

void InitAll(Local<Object> exports, Local<Object> module) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<Object> result = Object::New(isolate);
  NODE_SET_METHOD(result, "compose", Compose);
  NODE_SET_METHOD(result, "tesselate", Tssl);
  module->Set(String::NewFromUtf8(isolate, "exports"), result);
}

NODE_MODULE(addon, InitAll)
