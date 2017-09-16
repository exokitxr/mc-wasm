#include "noiser.h"
#include "v8-strings.h"
#include "cachedFastNoiseObject.h"
#include "fastNoiseObject.h"
#include "biomes.h"
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

const int NUM_CELLS = 16;
const int OVERSCAN = 1;
const int NUM_CELLS_OVERSCAN = NUM_CELLS + OVERSCAN;
const int NUM_CELLS_HEIGHT = 128;
const int NUM_CHUNKS_HEIGHT = NUM_CELLS_HEIGHT / NUM_CELLS;
const int NUM_CELLS_OVERSCAN_Y = NUM_CELLS_HEIGHT + OVERSCAN;

int getCoordOverscanIndex(int x, int z) {
  return x + z * NUM_CELLS_OVERSCAN;
}
int getEtherIndex(int x, int y, int z) {
  return x + (z * NUM_CELLS_OVERSCAN) + (y * NUM_CELLS_OVERSCAN * NUM_CELLS_OVERSCAN);
}

Persistent<Function> Noiser::constructor;
void Noiser::Init(Isolate* isolate) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(V8_STRINGS::Noiser.Get(isolate));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "getBiomeHeight", GetBiomeHeight);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getBiome", GetBiome);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getElevation", GetElevation);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getTemperature", GetTemperature);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillBiomes", FillBiomes);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillElevations", FillElevations);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillEther", FillEther);
  NODE_SET_PROTOTYPE_METHOD(tpl, "fillLiquid", FillLiquid);
  NODE_SET_PROTOTYPE_METHOD(tpl, "postProcessGeometry", PostProcessGeometry);

  constructor.Reset(isolate, tpl->GetFunction());
}
void Noiser::NewInstance(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  const unsigned argc = 1;
  Local<Value> argv[argc] = {args[0]};
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

  args.GetReturnValue().Set(instance);
}

void Noiser::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    Local<String> seedString = V8_STRINGS::seed.Get(isolate);

    Local<Object> opts = args[0]->ToObject();

    int seed = opts->Get(seedString)->Int32Value();

// std::cout << "got seed " << seed << "\n";

    Noiser* obj = new Noiser(seed);
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

Noiser::Noiser(int seed) :
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

void Noiser::GetBiome(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getBiome(args[0]->Int32Value(), args[1]->Int32Value())));
}

unsigned char Noiser::getBiome(int x, int z) {
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

void Noiser::GetBiomeHeight(const FunctionCallbackInfo<Value>& args) {
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

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getBiomeHeight((unsigned char)args[0]->Uint32Value(), args[1]->Int32Value(), args[2]->Int32Value())));
}

float Noiser::getBiomeHeight(unsigned char b, int x, int z) {
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

void Noiser::GetElevation(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getElevation(args[0]->Int32Value(), args[1]->Int32Value())));
}

float Noiser::getElevation(int x, int z) {
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

void Noiser::GetTemperature(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->getTemperature(args[0]->NumberValue(), args[1]->NumberValue())));
}

double Noiser::getTemperature(double x, double z) {
  return temperatureNoise.in2D(x, z);
}

void Noiser::FillBiomes(const FunctionCallbackInfo<Value>& args) {
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

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  obj->fillBiomes(args[0]->Int32Value(), args[1]->Int32Value(), biomes);
}

void Noiser::fillBiomes(int ox, int oz, unsigned char *biomes) {
  unsigned int index = 0;
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      biomes[index++] = getBiome((ox * NUM_CELLS) + x, (oz * NUM_CELLS) + z);
    }
  }
}

void Noiser::FillElevations(const FunctionCallbackInfo<Value>& args) {
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

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  obj->fillElevations(args[0]->Int32Value(), args[1]->Int32Value(), elevations);
}

void Noiser::fillElevations(int ox, int oz, float *elevations) {
  unsigned int index = 0;
  for (int z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (int x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      elevations[index++] = getElevation((ox * NUM_CELLS) + x, (oz * NUM_CELLS) + z);
    }
  }
}

void Noiser::FillEther(const FunctionCallbackInfo<Value>& args) {
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

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  obj->fillEther(elevations, ethers);
}

void Noiser::fillEther(float *elevations, float *ether) {
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

void Noiser::FillLiquid(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
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

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  obj->fillLiquid(ox, oz, ether, elevations, water, lava);
}

inline void setLiquid(int ox, int oz, int x, int y, int z, float *liquid) {
  x -= ox * NUM_CELLS;
  z -= oz * NUM_CELLS;

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
              liquid[index] = std::min<float>(-1.0 * (1.0 - (sqrt((float)dx*(float)dx + (float)dy*(float)dy + (float)dz*(float)dz) / (sqrt(3.0)*0.8))), liquid[index]);
            }
          }
        }
      }
    }
  }
}

void Noiser::fillLiquid(int ox, int oz, float *ether, float *elevations, float *water, float *lava) {
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

inline void postProcessGeometryRange(int ox, int oz, int start, int count, float *positions, float *colors, const std::function<void(const float,const float,const float,const float,const float,float &,float &,float &)>& getColor) {
  float *geometryPositions = positions + start;
  float *geometryColors = colors + start;

  unsigned int baseIndex = 0;
  for (int i = 0; i < count / 3; i++) {
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

void Noiser::PostProcessGeometry(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
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

  Noiser* obj = ObjectWrap::Unwrap<Noiser>(args.Holder());
  obj->postProcessGeometry(ox, oz, range, positions, colors, biomes);
}

void Noiser::postProcessGeometry(int ox, int oz, Local<Object> &range, float *positions, float *colors, unsigned char *biomes) {
  Isolate* isolate = Isolate::GetCurrent();

  Local<String> landStartString = V8_STRINGS::landStart.Get(isolate);
  Local<String> landCountString = V8_STRINGS::landCount.Get(isolate);
  Local<String> waterStartString = V8_STRINGS::waterStart.Get(isolate);
  Local<String> waterCountString = V8_STRINGS::waterCount.Get(isolate);
  Local<String> lavaStartString = V8_STRINGS::lavaStart.Get(isolate);
  Local<String> lavaCountString = V8_STRINGS::lavaCount.Get(isolate);

  int landStart = range->Get(landStartString)->Int32Value();
  int landCount = range->Get(landCountString)->Int32Value();
  postProcessGeometryRange(ox, oz, landStart, landCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
    const Biome &biome = BIOMES[biomes[getCoordOverscanIndex((int)std::floor(x - ox * NUM_CELLS), (int)std::floor(z - oz * NUM_CELLS))]];
    const unsigned int color = biome.color;
    r = ((color >> (8 * 2)) & 0xFF) / 255.0;
    g = ((color >> (8 * 1)) & 0xFF) / 255.0;
    b = ((color >> (8 * 0)) & 0xFF) / 255.0;
  });

  int waterStart = range->Get(waterStartString)->Int32Value();
  int waterCount = range->Get(waterCountString)->Int32Value();
  postProcessGeometryRange(ox, oz, waterStart, waterCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
    r = 0.0;
    g = 0.0;
    b = 1.0;
  });

  int lavaStart = range->Get(lavaStartString)->Int32Value();
  int lavaCount = range->Get(lavaCountString)->Int32Value();
  postProcessGeometryRange(ox, oz, lavaStart, lavaCount, positions, colors, [&](const float ox, const float oz, const float x, const float y, const float z, float &r, float &g, float &b)->void {
    r = 0.5;
    g = 0.0;
    b = 2.0;
  });
}
