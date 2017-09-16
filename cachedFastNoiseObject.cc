#include "cachedFastNoiseObject.h"
#include "fastNoiseObject.h"
#include "v8-strings.h"
// #include "MurmurHash3.h"
#include <node.h>
#include <cmath>
#include <random>
#include <unordered_map>
#include <vector>
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

const unsigned int NUM_CELLS = 16;

Persistent<Function> CachedFastNoiseObject::constructor;
void CachedFastNoiseObject::Init(Isolate* isolate) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(V8_STRINGS::CachedFastNoiseObject.Get(isolate));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "in2D", In2D);

  constructor.Reset(isolate, tpl->GetFunction());
}
void CachedFastNoiseObject::NewInstance(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  const unsigned argc = 1;
  Local<Value> argv[argc] = {args[0]};
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

  args.GetReturnValue().Set(instance);
}

CachedFastNoiseObject::CachedFastNoiseObject(int s, double frequency, int octaves) : FastNoiseObject(s, frequency, octaves) {
}

void CachedFastNoiseObject::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    Local<Object> opts = args[0]->ToObject();

    int seed = opts->Get(V8_STRINGS::seed.Get(isolate))->IntegerValue();
    double frequency = opts->Get(V8_STRINGS::frequency.Get(isolate))->NumberValue();
    int octaves = opts->Get(V8_STRINGS::octaves.Get(isolate))->IntegerValue();

    CachedFastNoiseObject* obj = new CachedFastNoiseObject(seed, frequency, octaves);
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

void CachedFastNoiseObject::In2D(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  CachedFastNoiseObject* obj = ObjectWrap::Unwrap<CachedFastNoiseObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->in2D(args[0]->NumberValue(), args[1]->NumberValue())));
}

double CachedFastNoiseObject::in2D(double x, double z) {
  const int ox = (int)std::floor(x / NUM_CELLS);
  const int oz = (int)std::floor(z / NUM_CELLS);
  const int ax = (int)std::floor(x) - ox * NUM_CELLS;
  const int az = (int)std::floor(z) - oz * NUM_CELLS;
  const int index = ax + az * NUM_CELLS;

  const std::pair<int, int> key(ox, oz);
  std::unordered_map<std::pair<int, int>, std::vector<double>>::iterator entryIter = cache.find(key);

  if (entryIter != cache.end()) {
    return entryIter->second[index];
  } else {
    std::vector<double> &entry = cache[key];
    entry.reserve(NUM_CELLS * NUM_CELLS);

    for (unsigned int dz = 0; dz < NUM_CELLS; dz++) {
      for (unsigned int dx = 0; dx < NUM_CELLS; dx++) {
        entry[dx + dz * NUM_CELLS] = FastNoiseObject::in2D(ox * NUM_CELLS + dx, oz * NUM_CELLS + dz);
      }
    }

    return entry[index];
  }
}

/* CachedFastNoiseObject elevationNoise1(rng(), 2, 1);
CachedFastNoiseObject elevationNoise2(rng(), 2, 1);
CachedFastNoiseObject elevationNoise3(rng(), 2, 1);
CachedFastNoiseObject wormNoise(rng(), 0.002, 4);
CachedFastNoiseObject oceanNoise(rng(), 0.002, 4);
CachedFastNoiseObject riverNoise(rng(), 0.002, 4);
CachedFastNoiseObject temperatureNoise(rng(), 0.002, 4);
CachedFastNoiseObject humidityNoise(rng(), 0.002, 4);

const _getCachesIndex2D = (x, z) => mod(x, 256) | mod(z, 256) << 8; // XXX make sure this does not overflow cache size
const _getCacheIndex2D = (x, z) => x + z * NUM_CELLS;
const _makeCacher2D = (gen, {type = Float32Array} = {}) => {
  const caches = {};
  return (x, z) => {
    const ox = Math.floor(x / NUM_CELLS);
    const oz = Math.floor(z / NUM_CELLS);
    const cachesIndex = _getCachesIndex2D(ox, oz);
    let entry = caches[cachesIndex];
    if (entry === undefined) {
      entry = new type(NUM_CELLS * NUM_CELLS);
      let index = 0;
      for (let dz = 0; dz < NUM_CELLS; dz++) {
        for (let dx = 0; dx < NUM_CELLS; dx++) {
          entry[index++] = gen(ox * NUM_CELLS + dx, oz * NUM_CELLS + dz);
        }
      }
      caches[cachesIndex] = entry;
    }
    return entry[_getCacheIndex2D(x - ox * NUM_CELLS, z - oz * NUM_CELLS)];
  };
};

const _getCachesIndex3D = (b, x, z) => mod(b, 256) | mod(x, 256) << 8 | mod(z, 256) << 16; // XXX make sure this does not overflow cache size
const _getCacheIndex3D = (x, z) => x + z * NUM_CELLS;
const _makeCacher3D = (gen, {type = Float32Array} = {}) => {
  const caches = {};
  return (b, x, z) => {
    const ox = Math.floor(x / NUM_CELLS);
    const oz = Math.floor(z / NUM_CELLS);
    const cachesIndex = _getCachesIndex3D(b, ox, oz);
    let entry = caches[cachesIndex];
    if (entry === undefined) {
      entry = new type(NUM_CELLS * NUM_CELLS);
      let index = 0;
      for (let dz = 0; dz < NUM_CELLS; dz++) {
        for (let dx = 0; dx < NUM_CELLS; dx++) {
          entry[index++] = gen(b, ox * NUM_CELLS + dx, oz * NUM_CELLS + dz);
        }
      }
      caches[cachesIndex] = entry;
    }
    return entry[_getCacheIndex3D(x - ox * NUM_CELLS, z - oz * NUM_CELLS)];
  };
};

const _getBiome = _makeCacher2D((x, z) => {
  let biome;
  const _genOcean = () => {
    if (_random.oceanNoise(x + 1000, z + 1000) < (90 / 255)) {
      biome = BIOMES.biOcean.index;
    }
  };
  const _genRivers = () => {
    if (biome === undefined) {
      const n = _random.riverNoise(x + 1000, z + 1000);
      const range = 0.04;
      if (n > 0.5 - range && n < 0.5 + range) {
        biome = BIOMES.biRiver.index;
      }
    }
  };
  const _genFreezeWater = () => {
    if (_random.temperatureNoise(x + 1000, z + 1000) < ((4 * 16) / 255)) {
      if (biome === BIOMES.biOcean.index) {
        biome = BIOMES.biFrozenOcean.index;
      } else if (biome === BIOMES.biRiver.index) {
        biome = BIOMES.biFrozenRiver.index;
      }
    }
  };
  const _genLand = () => {
    if (biome === undefined) {
      const t = Math.floor(_random.temperatureNoise(x + 1000, z + 1000) * 16);
      const h = Math.floor(_random.humidityNoise(x + 1000, z + 1000) * 16);
      biome = BIOMES_TEMPERATURE_HUMIDITY[t + 16 * h].index;
    }
  };
  _genOcean();
  _genRivers();
  _genFreezeWater();
  _genLand();

  return biome;
}, {type: Uint8Array});

const _getBiomeHeight = _makeCacher3D((b, x, z) => {
  const biome = BIOMES_INDEX[b];
  return biome.baseHeight +
    _random.elevationNoise1.in2D(x * biome.amps[0][0], z * biome.amps[0][0]) * biome.amps[0][1] +
    _random.elevationNoise2.in2D(x * biome.amps[1][0], z * biome.amps[1][0]) * biome.amps[1][1] +
    _random.elevationNoise3.in2D(x * biome.amps[2][0], z * biome.amps[2][0]) * biome.amps[2][1];
});

const _getElevation = _makeCacher2D((x, z) => {
  const biomeCounts = {};
  let totalBiomeCounts = 0;
  for (let dz = -8; dz <= 8; dz++) {
    for (let dx = -8; dx <= 8; dx++) {
      const biome = _getBiome(x + dx, z + dz);
      let biomeCount = biomeCounts[biome];
      if (!biomeCount) {
        biomeCount = {
          count: 0,
          height: _getBiomeHeight(biome, x, z),
        };
        biomeCounts[biome] = biomeCount;
      }
      biomeCount.count++;
      totalBiomeCounts++;
    }
  }

  let elevationSum = 0;
  for (const index in biomeCounts) {
    const biomeCount = biomeCounts[index];
    elevationSum += biomeCount.count * biomeCount.height;
  }
  return elevationSum / totalBiomeCounts;
});

const biomes = (() => {
  let biomes = opts.oldBiomes;
  if (!biomes) {
    biomes = new Uint8Array(NUM_CELLS_OVERSCAN * NUM_CELLS_OVERSCAN);
    let index = 0;
    for (let z = 0; z < NUM_CELLS_OVERSCAN; z++) {
      for (let x = 0; x < NUM_CELLS_OVERSCAN; x++) {
        biomes[index++] = _getBiome((ox * NUM_CELLS) + x, (oy * NUM_CELLS) + z);
      }
    }
  }
  return biomes;
})();

let elevations = opts.oldElevations;
if (!elevations) {
  elevations = new Float32Array(NUM_CELLS_OVERSCAN * NUM_CELLS_OVERSCAN);
  let index = 0;
  for (let z = 0; z < NUM_CELLS_OVERSCAN; z++) {
    for (let x = 0; x < NUM_CELLS_OVERSCAN; x++) {
      elevations[index++] = _getElevation((ox * NUM_CELLS) + x, (oy * NUM_CELLS) + z);
    }
  }
} */
