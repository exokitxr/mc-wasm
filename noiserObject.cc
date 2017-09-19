#include "noiserObject.h"
#include "util.h"
#include "v8-strings.h"
#include "biomes.h"
#include "cachedNoiseObject.h"
#include "noiseObject.h"
#include "march.h"
#include "heightfield.h"
#include "flod.h"
#include <node.h>
#include <cmath>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <functional>
// #include <iostream>

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::Array;
using v8::ArrayBuffer;

Persistent<Function> NoiserObject::constructor;
void NoiserObject::Init(Isolate* isolate) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(V8_STRINGS::Noiser.Get(isolate));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "getBiomeHeight", GetBiomeHeight);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getBiome", GetBiome);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getElevation", GetElevation);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getTemperature", GetTemperature);
  /* NODE_SET_PROTOTYPE_METHOD(tpl, "fillBiomes", FillBiomes);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillElevations", FillElevations);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillEther", FillEther);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillLiquid", FillLiquid);
  NODE_SET_PROTOTYPE_METHOD(tpl, "applyEther", ApplyEther); */
  NODE_SET_PROTOTYPE_METHOD(tpl, "fill", Fill);
  // NODE_SET_PROTOTYPE_METHOD(tpl, "postProcessGeometry", PostProcessGeometry);

  constructor.Reset(isolate, tpl->GetFunction());
}
void NoiserObject::NewInstance(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  const unsigned argc = 1;
  Local<Value> argv[argc] = {args[0]};
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

  args.GetReturnValue().Set(instance);
}

void NoiserObject::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    Local<String> seedString = V8_STRINGS::seed.Get(isolate);

    Local<Object> opts = args[0]->ToObject();

    int seed = opts->Get(seedString)->Int32Value();

// std::cout << "got seed " << seed << "\n";

    NoiserObject* obj = new NoiserObject(seed);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = {args[0]};
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
  }
}

NoiserObject::NoiserObject(int seed) :
  rng(seed),
  elevationNoise1(rng(), 2, 1),
  elevationNoise2(rng(), 2, 1),
  elevationNoise3(rng(), 2, 1),
  wormNoise(rng(), 0.002, 4),
  oceanNoise(rng(), 0.002, 4),
  riverNoise(rng(), 0.002, 4),
  temperatureNoise(rng(), 0.002, 4),
  humidityNoise(rng(), 0.002, 4)
{}

void NoiserObject::GetBiome(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getBiome(args[0]->Int32Value(), args[1]->Int32Value())));
}

unsigned char NoiserObject::getBiome(int x, int z) {
  const std::pair<int, int> key(x, z);
  std::unordered_map<std::pair<int, int>, unsigned char>::iterator entryIter = biomeCache.find(key);

  if (entryIter != biomeCache.end()) {
    return entryIter->second;
  } else {
    unsigned char &biome = biomeCache[key];

    biome = 0xFF;
    if (oceanNoise.in2D(x + 1000, z + 1000) < (90.0 / 255.0)) {
      biome = (unsigned char)BIOME::biOcean;
    }
    if (biome == 0xFF) {
      const double n = riverNoise.in2D(x + 1000, z + 1000);
      const double range = 0.04;
      if (n > 0.5 - range && n < 0.5 + range) {
        biome = (unsigned char)BIOME::biRiver;
      }
    }
    if (temperatureNoise.in2D(x + 1000, z + 1000) < ((4.0 * 16.0) / 255.0)) {
      if (biome == (unsigned char)BIOME::biOcean) {
        biome = (unsigned char)BIOME::biFrozenOcean;
      } else if (biome == (unsigned char)BIOME::biRiver) {
        biome = (unsigned char)BIOME::biFrozenRiver;
      }
    }
    if (biome == 0xFF) {
      const int t = (int)std::floor(temperatureNoise.in2D(x + 1000, z + 1000) * 16.0);
      const int h = (int)std::floor(humidityNoise.in2D(x + 1000, z + 1000) * 16.0);
      biome = (unsigned char)BIOMES_TEMPERATURE_HUMIDITY[t + 16 * h];
    }

    return biome;
  }
}

void NoiserObject::GetBiomeHeight(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }

  // Check the argument types
  if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getBiomeHeight((unsigned char)args[0]->Uint32Value(), args[1]->Int32Value(), args[2]->Int32Value())));
}

float NoiserObject::getBiomeHeight(unsigned char b, int x, int z) {
  const std::tuple<unsigned char, int, int> key(b, x, z);
  std::unordered_map<std::tuple<unsigned char, int, int>, float>::iterator entryIter = biomeHeightCache.find(key);

  if (entryIter != biomeHeightCache.end()) {
    return entryIter->second;
  } else {
    float &biomeHeight = biomeHeightCache[key];

    const Biome &biome = BIOMES[b];
    biomeHeight = biome.baseHeight +
      elevationNoise1.in2D(x * biome.amps[0][0], z * biome.amps[0][0]) * biome.amps[0][1] +
      elevationNoise2.in2D(x * biome.amps[1][0], z * biome.amps[1][0]) * biome.amps[1][1] +
      elevationNoise3.in2D(x * biome.amps[2][0], z * biome.amps[2][0]) * biome.amps[2][1];

    return biomeHeight;
  }
}

void NoiserObject::GetElevation(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getElevation(args[0]->Int32Value(), args[1]->Int32Value())));
}

float NoiserObject::getElevation(int x, int z) {
  const std::pair<int, int> key(x, z);
  std::unordered_map<std::pair<int, int>, float>::iterator entryIter = elevationCache.find(key);

  if (entryIter != elevationCache.end()) {
    return entryIter->second;
  } else {
    float &elevation = elevationCache[key];

    std::unordered_map<unsigned char, unsigned int> biomeCounts;
    for (int dz = -8; dz <= 8; dz++) {
      for (int dx = -8; dx <= 8; dx++) {
        biomeCounts[getBiome(x + dx, z + dz)]++;
      }
    }

    float elevationSum = 0;
    for (auto const &iter : biomeCounts) {
      elevationSum += iter.second * getBiomeHeight(iter.first, x, z);
    }
    elevation = elevationSum / ((8 * 2 + 1) * (8 * 2 + 1));

    return elevation;
  }
}

void NoiserObject::GetTemperature(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getTemperature(args[0]->NumberValue(), args[1]->NumberValue())));
}

double NoiserObject::getTemperature(double x, double z) {
  return temperatureNoise.in2D(x, z);
}

/* void NoiserObject::FillBiomes(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }

  // Check the argument types
  if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsUint8Array()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);
  Local<ArrayBuffer> biomesBuffer = Local<ArrayBuffer>::Cast(args[2]->ToObject()->Get(bufferString));
  unsigned int biomesByteOffset = args[2]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned char *biomes = (unsigned char *)((char *)biomesBuffer->GetContents().Data() + biomesByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  obj->fillBiomes(args[0]->Int32Value(), args[1]->Int32Value(), biomes);
} */

void NoiserObject::fillBiomes(int ox, int oz, unsigned char *biomes) {
  unsigned int index = 0;
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      biomes[index++] = getBiome((ox * NUM_CELLS) + x, (oz * NUM_CELLS) + z);
    }
  }
}

/* void NoiserObject::FillElevations(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsFloat32Array()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);
  Local<ArrayBuffer> elevationsBuffer = Local<ArrayBuffer>::Cast(args[2]->ToObject()->Get(bufferString));
  unsigned int elevationsByteOffset = args[2]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *elevations = (float *)((char *)elevationsBuffer->GetContents().Data() + elevationsByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  obj->fillElevations(args[0]->Int32Value(), args[1]->Int32Value(), elevations);
} */

void NoiserObject::fillElevations(int ox, int oz, float *elevations) {
  unsigned int index = 0;
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      elevations[index++] = getElevation((ox * NUM_CELLS) + x, (oz * NUM_CELLS) + z);
    }
  }
}

/* void NoiserObject::FillEther(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsFloat32Array() || !args[1]->IsFloat32Array()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);
  Local<ArrayBuffer> elevationsBuffer = Local<ArrayBuffer>::Cast(args[0]->ToObject()->Get(bufferString));
  unsigned int elevationsByteOffset = args[0]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *elevations = (float *)((char *)elevationsBuffer->GetContents().Data() + elevationsByteOffset);
  Local<ArrayBuffer> ethersBuffer = Local<ArrayBuffer>::Cast(args[1]->ToObject()->Get(bufferString));
  unsigned int ethersByteOffset = args[1]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *ethers = (float *)((char *)ethersBuffer->GetContents().Data() + ethersByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  obj->fillEther(elevations, ethers);
} */

void NoiserObject::fillEther(float *elevations, float *ether) {
  unsigned int index = 0;
  for (int y = 0; y < NUM_CELLS_OVERSCAN_Y; y++) {
    for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
      for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
        const float elevation = elevations[x + z * NUM_CELLS_OVERSCAN];
        ether[index++] = std::min<float>(std::max<float>((float)y - elevation, -1.0), 1.0);
      }
    }
  }
}

/* void NoiserObject::FillLiquid(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 6) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsFloat32Array() || !args[3]->IsFloat32Array() || !args[4]->IsFloat32Array() || !args[5]->IsFloat32Array()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);
  int ox = args[0]->Int32Value();
  int oz = args[1]->Int32Value();

  Local<ArrayBuffer> etherBuffer = Local<ArrayBuffer>::Cast(args[2]->ToObject()->Get(bufferString));
  unsigned int etherByteOffset = args[2]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *ether = (float *)((char *)etherBuffer->GetContents().Data() + etherByteOffset);

  Local<ArrayBuffer> elevationsBuffer = Local<ArrayBuffer>::Cast(args[3]->ToObject()->Get(bufferString));
  unsigned int elevationsByteOffset = args[3]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *elevations = (float *)((char *)elevationsBuffer->GetContents().Data() + elevationsByteOffset);

  Local<ArrayBuffer> waterBuffer = Local<ArrayBuffer>::Cast(args[4]->ToObject()->Get(bufferString));
  unsigned int waterByteOffset = args[4]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *water = (float *)((char *)waterBuffer->GetContents().Data() + waterByteOffset);

  Local<ArrayBuffer> lavaBuffer = Local<ArrayBuffer>::Cast(args[5]->ToObject()->Get(bufferString));
  unsigned int lavaByteOffset = args[5]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *lava = (float *)((char *)lavaBuffer->GetContents().Data() + lavaByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  obj->fillLiquid(ox, oz, ether, elevations, water, lava);
} */

inline void setLiquid(int ox, int oz, int x, int y, int z, float *liquid) {
  x -= ox * NUM_CELLS;
  z -= oz * NUM_CELLS;

  const float factor = sqrt(3.0) * 0.8;

  for (int dz = -1; dz <= 1; dz++) {
    const int az = z + dz;
    if (az >= 0 && az <= NUM_CELLS) {
      for (int dx = -1; dx <= 1; dx++) {
        const int ax = x + dx;
        if (ax >= 0 && ax <= NUM_CELLS) {
          for (int dy = -1; dy <= 1; dy++) {
            const int ay = y + dy;
            if (ay >= 0 && ay < (NUM_CELLS_HEIGHT + 1)) {
              const int index = getEtherIndex(ax, ay, az);
              liquid[index] = std::min<float>(-1.0 * (1.0 - (sqrt((float)dx*(float)dx + (float)dy*(float)dy + (float)dz*(float)dz) / factor)), liquid[index]);
            }
          }
        }
      }
    }
  }
}

void NoiserObject::fillLiquid(int ox, int oz, float *ether, float *elevations, float *water, float *lava) {
  for (unsigned int i = 0; i < (NUM_CELLS + 1) * (NUM_CELLS_HEIGHT + 1) * (NUM_CELLS + 1); i++) {
    water[i] = 1.0;
    lava[i] = 1.0;
  }

  // water
  unsigned int index = 0;
  for (int z = 0; z <= NUM_CELLS; z++) {
    for (int x = 0; x <= NUM_CELLS; x++) {
      const float elevation = elevations[index++];
      for (int y = elevation; y < 64; y++) {
        if (y < 64 && y >= elevation) {
          const int index = getEtherIndex(x, y, z);
          water[index] = ether[index] * -1;
        }
      }
    }
  }

  // lava
  index = 0;
  for (int doz = -1; doz <= 1; doz++) {
    for (int dox = -1; dox <= 1; dox++) {
      for (int z = 0; z <= NUM_CELLS; z++) {
        for (int x = 0; x <= NUM_CELLS; x++) {
          const int ax = ((ox + dox) * NUM_CELLS) + x;
          const int az = ((oz + doz) * NUM_CELLS) + z;
          const float elevation = getElevation(ax, az);

          if (elevation >= 80 && getTemperature(ax + 1000, az + 1000) < 0.235) {
            setLiquid(ox, oz, ax, (int)std::floor(elevation + 1.0), az, lava);
          }
        }
      }
    }
  }
}

/* void NoiserObject::ApplyEther(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsFloat32Array() || !args[1]->IsNumber() || !args[2]->IsFloat32Array()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);

  Local<ArrayBuffer> newEtherBuffer = Local<ArrayBuffer>::Cast(args[0]->ToObject()->Get(bufferString));
  unsigned int newEtherByteOffset = args[0]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *newEther = (float *)((char *)newEtherBuffer->GetContents().Data() + newEtherByteOffset); 
  unsigned int numNewEthers = args[1]->Uint32Value();

  Local<ArrayBuffer> etherBuffer = Local<ArrayBuffer>::Cast(args[2]->ToObject()->Get(bufferString));
  unsigned int etherByteOffset = args[2]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *ether = (float *)((char *)etherBuffer->GetContents().Data() + etherByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  obj->applyEther(newEther, numNewEthers, ether);
} */

void NoiserObject::applyEther(float *newEther, unsigned int numNewEthers, float *ether) {
  unsigned int baseIndex = 0;
  for (unsigned int i = 0; i < numNewEthers; i++) {
    const float x = newEther[baseIndex + 0];
    const float y = newEther[baseIndex + 1];
    const float z = newEther[baseIndex + 2];
    const float v = newEther[baseIndex + 3];
    for (int dz = -HOLE_SIZE; dz <= HOLE_SIZE; dz++) {
      const int az = z + dz;
      if (az >= 0 && az < (NUM_CELLS + 1)) {
        for (int dx = -HOLE_SIZE; dx <= HOLE_SIZE; dx++) {
          const int ax = x + dx;
          if (ax >= 0 && ax < (NUM_CELLS + 1)) {
            for (int dy = -HOLE_SIZE; dy <= HOLE_SIZE; dy++) {
              const int ay = y + dy;
              if (ay >= 0 && ay < (NUM_CELLS_HEIGHT + 1)) {
                ether[getEtherIndex(ax, ay, az)] += v * std::max<float>((float)HOLE_SIZE - std::sqrt((float)dx * (float)dx + (float)dy * (float)dy + (float)dz * (float)dz), 0) / std::sqrt((float)HOLE_SIZE * (float)HOLE_SIZE * 3.0);
              }
            }
          }
        }
      }
    }
    baseIndex += 4;
  }
}

void NoiserObject::makeGeometries(int ox, int oy, float *ether, float *water, float *lava, float *positionsBuffer, unsigned int *indicesBuffer, unsigned int *attributeRanges, unsigned int *indexRanges) {
  int attributeIndex = 0;
  int indexIndex = 0;

  // land
  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    unsigned int positionIndex;
    unsigned int faceIndex;

    int dims[3] = {
      NUM_CELLS + 1,
      NUM_CELLS + 1,
      NUM_CELLS + 1
    };
    float *potential = ether;
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    int indexOffset = attributeIndex / 3;
    float *positions = (float *)((char *)positionsBuffer + attributeIndex * 4);
    unsigned int *faces = (unsigned int *)((char *)indicesBuffer + indexIndex * 4);

    marchingCubes(dims, potential, shift, indexOffset, positions, faces, positionIndex, faceIndex);

    attributeRanges[i * 6 + 0] = attributeIndex;
    attributeRanges[i * 6 + 1] = positionIndex;
    indexRanges[i * 6 + 0] = indexIndex;
    indexRanges[i * 6 + 1] = faceIndex,

    attributeIndex += positionIndex;
    indexIndex += faceIndex;
  }

  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
{
    unsigned int positionIndex;
    unsigned int faceIndex;

    // water
    int dims[3] = {
      NUM_CELLS + 1,
      NUM_CELLS + 1,
      NUM_CELLS + 1
    };
    float *potential = water;
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    int indexOffset = attributeIndex / 3;
    float *positions = (float *)((char *)positionsBuffer + attributeIndex * 4);
    unsigned int *faces = (unsigned int *)((char *)indicesBuffer + indexIndex * 4);

    marchingCubes(dims, potential, shift, indexOffset, positions, faces, positionIndex, faceIndex);

    attributeRanges[i * 6 + 2] = attributeIndex;
    attributeRanges[i * 6 + 3] = positionIndex;
    indexRanges[i * 6 + 2] = indexIndex;
    indexRanges[i * 6 + 3] = faceIndex;

    attributeIndex += positionIndex;
    indexIndex += faceIndex;
}
{
    unsigned int positionIndex;
    unsigned int faceIndex;

    // lava
    int dims[3] = {
      NUM_CELLS + 1,
      NUM_CELLS + 1,
      NUM_CELLS + 1
    };
    float *potential = lava;
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    int indexOffset = attributeIndex / 3;
    float *positions = (float *)((char *)positionsBuffer + attributeIndex * 4);
    unsigned int *faces = (unsigned int *)((char *)indicesBuffer + indexIndex * 4);

    marchingCubes(dims, potential, shift, indexOffset, positions, faces, positionIndex, faceIndex);

    attributeRanges[i * 6 + 4] = attributeIndex;
    attributeRanges[i * 6 + 5] = positionIndex;
    indexRanges[i * 6 + 4] = indexIndex;
    indexRanges[i * 6 + 5] = faceIndex;

    attributeIndex += positionIndex;
    indexIndex += faceIndex;
}
  }
}


void NoiserObject::Fill(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 21) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }

  // Check the argument types
  if (
    !args[0]->IsNumber() ||
    !args[1]->IsNumber() ||
    !args[2]->IsUint8Array() ||
    !args[3]->IsBoolean() ||
    !args[4]->IsFloat32Array() ||
    !args[5]->IsBoolean() ||
    !args[6]->IsFloat32Array() ||
    !args[7]->IsBoolean() ||
    !args[8]->IsFloat32Array() ||
    !args[9]->IsFloat32Array() ||
    !args[10]->IsBoolean() ||
    !args[11]->IsFloat32Array() ||
    !args[12]->IsNumber() ||
    !args[13]->IsFloat32Array() ||
    !args[14]->IsUint32Array() ||
    !args[15]->IsUint32Array() ||
    !args[16]->IsUint32Array() ||
    !args[17]->IsFloat32Array() ||
    !args[18]->IsFloat32Array() ||
    !args[19]->IsFloat32Array() ||
    !args[20]->IsUint8Array()
  ) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);

  int ox = args[0]->Int32Value();
  int oz = args[1]->Int32Value();

  Local<ArrayBuffer> biomesBuffer = Local<ArrayBuffer>::Cast(args[2]->ToObject()->Get(bufferString));
  unsigned int biomesByteOffset = args[2]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned char *biomes = (unsigned char *)((char *)biomesBuffer->GetContents().Data() + biomesByteOffset);

  bool fillBiomes = args[3]->BooleanValue();

  Local<ArrayBuffer> elevationsBuffer = Local<ArrayBuffer>::Cast(args[4]->ToObject()->Get(bufferString));
  unsigned int elevationsByteOffset = args[4]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *elevations = (float *)((char *)elevationsBuffer->GetContents().Data() + elevationsByteOffset);

  bool fillElevations = args[5]->BooleanValue();

  Local<ArrayBuffer> ethersBuffer = Local<ArrayBuffer>::Cast(args[6]->ToObject()->Get(bufferString));
  unsigned int ethersByteOffset = args[6]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *ethers = (float *)((char *)ethersBuffer->GetContents().Data() + ethersByteOffset);

  bool fillEther = args[7]->BooleanValue();

  Local<ArrayBuffer> waterBuffer = Local<ArrayBuffer>::Cast(args[8]->ToObject()->Get(bufferString));
  unsigned int waterByteOffset = args[8]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *water = (float *)((char *)waterBuffer->GetContents().Data() + waterByteOffset);

  Local<ArrayBuffer> lavaBuffer = Local<ArrayBuffer>::Cast(args[9]->ToObject()->Get(bufferString));
  unsigned int lavaByteOffset = args[9]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *lava = (float *)((char *)lavaBuffer->GetContents().Data() + lavaByteOffset);

  bool fillLiquid = args[10]->BooleanValue();

  Local<ArrayBuffer> newEtherBuffer = Local<ArrayBuffer>::Cast(args[11]->ToObject()->Get(bufferString));
  unsigned int newEtherByteOffset = args[11]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *newEther = (float *)((char *)newEtherBuffer->GetContents().Data() + newEtherByteOffset);

  unsigned int numNewEthers = args[12]->Uint32Value();

  Local<ArrayBuffer> positionsBuffer = Local<ArrayBuffer>::Cast(args[13]->ToObject()->Get(bufferString));
  unsigned int positionsByteOffset = args[13]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *positions = (float *)((char *)positionsBuffer->GetContents().Data() + positionsByteOffset);

  Local<ArrayBuffer> indicesBuffer = Local<ArrayBuffer>::Cast(args[14]->ToObject()->Get(bufferString));
  unsigned int indicesByteOffset = args[14]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned int *indices = (unsigned int *)((char *)indicesBuffer->GetContents().Data() + indicesByteOffset);

  Local<ArrayBuffer> attributeRangeBuffer = Local<ArrayBuffer>::Cast(args[15]->ToObject()->Get(bufferString));
  unsigned int attributeRangeByteOffset = args[15]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned int *attributeRanges = (unsigned int *)((char *)attributeRangeBuffer->GetContents().Data() + attributeRangeByteOffset);

  Local<ArrayBuffer> indexRangeBuffer = Local<ArrayBuffer>::Cast(args[16]->ToObject()->Get(bufferString));
  unsigned int indexRangeByteOffset = args[16]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned int *indexRanges = (unsigned int *)((char *)indexRangeBuffer->GetContents().Data() + indexRangeByteOffset);

  Local<ArrayBuffer> heightfieldBuffer = Local<ArrayBuffer>::Cast(args[17]->ToObject()->Get(bufferString));
  unsigned int heightfieldByteOffset = args[17]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *heightfield = (float *)((char *)heightfieldBuffer->GetContents().Data() + heightfieldByteOffset);

  Local<ArrayBuffer> staticHeightfieldBuffer = Local<ArrayBuffer>::Cast(args[18]->ToObject()->Get(bufferString));
  unsigned int staticHeightfieldByteOffset = args[18]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *staticHeightfield = (float *)((char *)staticHeightfieldBuffer->GetContents().Data() + staticHeightfieldByteOffset);

  Local<ArrayBuffer> colorsBuffer = Local<ArrayBuffer>::Cast(args[19]->ToObject()->Get(bufferString));
  unsigned int colorsByteOffset = args[19]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *colors = (float *)((char *)colorsBuffer->GetContents().Data() + colorsByteOffset);

  Local<ArrayBuffer> peeksBuffer = Local<ArrayBuffer>::Cast(args[20]->ToObject()->Get(bufferString));
  unsigned int peeksByteOffset = args[20]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned char *peeks = (unsigned char *)((char *)peeksBuffer->GetContents().Data() + peeksByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  if (fillBiomes) {
    obj->fillBiomes(ox, oz, biomes);
  }
  if (fillElevations) {
    obj->fillElevations(ox, oz, elevations);
  }
  if (fillEther) {
    obj->fillEther(elevations, ethers);
  }
  if (fillLiquid) {
    obj->fillLiquid(ox, oz, ethers, elevations, water, lava);
  }
  if (numNewEthers > 0) {
    obj->applyEther(newEther, numNewEthers, ethers);
  }

  obj->makeGeometries(ox, oz, ethers, water, lava, positions, indices, attributeRanges, indexRanges);

  unsigned int numIndices = indexRanges[5 * 6 + 4] + indexRanges[5 * 6 + 5];
  genHeightfield(positions, indices, numIndices, heightfield, staticHeightfield);

  obj->postProcessGeometry(ox, oz, attributeRanges, positions, colors, biomes);

  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    int shift[3] = {
      0,
      NUM_CELLS * i,
      0
    };
    flod(ethers, shift, peeks + i * 16);
  }
}

inline void postProcessGeometryRange(int ox, int oz, unsigned int start, unsigned int count, float *positions, float *colors, const std::function<void(const float,const float,const float,const float,const float,float &,float &,float &)>& getColor) {
  float *geometryPositions = positions + start;
  float *geometryColors = colors + start;

  unsigned int baseIndex = 0;
  for (unsigned int i = 0; i < count / 3; i++) {
    const float x = geometryPositions[baseIndex + 0];
    const float y = geometryPositions[baseIndex + 1];
    const float z = geometryPositions[baseIndex + 2];

    const float ax = x + (ox * NUM_CELLS);
    const float ay = y;
    const float az = z + (oz * NUM_CELLS);

    geometryPositions[baseIndex + 0] = ax;
    geometryPositions[baseIndex + 2] = az;

    getColor(ox, oz, ax, ay, az, geometryColors[baseIndex + 0], geometryColors[baseIndex + 1], geometryColors[baseIndex + 2]);

    baseIndex += 3;
  }
};

/* void NoiserObject::PostProcessGeometry(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 6) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsObject() || !args[3]->IsFloat32Array() || !args[4]->IsFloat32Array() || !args[5]->IsUint8Array()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Local<String> bufferString = V8_STRINGS::buffer.Get(isolate);
  Local<String> byteOffsetString = V8_STRINGS::byteOffset.Get(isolate);

  int ox = args[0]->Int32Value();
  int oz = args[1]->Int32Value();
  Local<Object> range = args[2]->ToObject();
  Local<ArrayBuffer> positionsBuffer = Local<ArrayBuffer>::Cast(args[3]->ToObject()->Get(bufferString));
  unsigned int positionsByteOffset = args[3]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *positions = (float *)((char *)positionsBuffer->GetContents().Data() + positionsByteOffset);
  Local<ArrayBuffer> colorsBuffer = Local<ArrayBuffer>::Cast(args[4]->ToObject()->Get(bufferString));
  unsigned int colorsByteOffset = args[4]->ToObject()->Get(byteOffsetString)->Uint32Value();
  float *colors = (float *)((char *)colorsBuffer->GetContents().Data() + colorsByteOffset);
  Local<ArrayBuffer> biomesBuffer = Local<ArrayBuffer>::Cast(args[5]->ToObject()->Get(bufferString));
  unsigned int biomesByteOffset = args[5]->ToObject()->Get(byteOffsetString)->Uint32Value();
  unsigned char *biomes = (unsigned char *)((char *)biomesBuffer->GetContents().Data() + biomesByteOffset);

  NoiserObject* obj = ObjectWrap::Unwrap<NoiserObject>(args.Holder());
  obj->postProcessGeometry(ox, oz, range, positions, colors, biomes);
} */

void NoiserObject::postProcessGeometry(int ox, int oz, unsigned int *attributeRanges, float *positions, float *colors, unsigned char *biomes) {
  for (int i = 0; i < NUM_CHUNKS_HEIGHT; i++) {
    unsigned int landStart = attributeRanges[i * 6 + 0];
    unsigned int landCount = attributeRanges[i * 6 + 1];
// std::cout << "invoke 1 " << landStart << " : " << landCount << "\n";
    postProcessGeometryRange(ox, oz, landStart, landCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
      const Biome &biome = BIOMES[biomes[getCoordOverscanIndex((int)x - ox * NUM_CELLS, (int)z - oz * NUM_CELLS)]];
      const unsigned int color = biome.color;
      r = ((color >> (8 * 2)) & 0xFF) / 255.0;
      g = ((color >> (8 * 1)) & 0xFF) / 255.0;
      b = ((color >> (8 * 0)) & 0xFF) / 255.0;
    });

    unsigned int waterStart = attributeRanges[i * 6 + 2];
    unsigned int waterCount = attributeRanges[i * 6 + 3];
// std::cout << "invoke 2 " << waterStart << " : " << waterCount << "\n";
    postProcessGeometryRange(ox, oz, waterStart, waterCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
      r = 0.0;
      g = 0.0;
      b = 1.0;
    });

    unsigned int lavaStart = attributeRanges[i * 6 + 4];
    unsigned int lavaCount = attributeRanges[i * 6 + 5];
// std::cout << "invoke 3 " << lavaStart << " : " << lavaCount << "\n";
    postProcessGeometryRange(ox, oz, lavaStart, lavaCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
      r = 0.5;
      g = 0.0;
      b = 2.0;
    });
  }
}
