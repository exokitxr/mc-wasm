#ifndef NOISER_OBJECT_H
#define NOISER_OBJECT_H

#include "noiseObject.h"
#include "cachedNoiseObject.h"
#include "hash.h"
#include <node.h>
#include <random>
#include <unordered_map>

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
using v8::Object;
using v8::Array;

class NoiserObject : public node::ObjectWrap {
  public:
    static Persistent<Function> constructor;
    static void Init(Isolate* isolate);
    static void NewInstance(const FunctionCallbackInfo<Value>& args);

    std::mt19937 rng;
    NoiseObject elevationNoise1;
    NoiseObject elevationNoise2;
    NoiseObject elevationNoise3;
    CachedNoiseObject wormNoise;
    CachedNoiseObject oceanNoise;
    CachedNoiseObject riverNoise;
    CachedNoiseObject temperatureNoise;
    CachedNoiseObject humidityNoise;

    std::unordered_map<std::pair<int, int>, unsigned char> biomeCache;
    std::unordered_map<std::tuple<unsigned char, int, int>, float> biomeHeightCache;
    std::unordered_map<std::pair<int, int>, float> elevationCache;

    explicit NoiserObject(int seed);

    static void New(const FunctionCallbackInfo<Value>& args);
    static void GetBiome(const FunctionCallbackInfo<Value>& args);
    unsigned char getBiome(int x, int z);
    static void GetBiomeHeight(const FunctionCallbackInfo<Value>& args);
    float getBiomeHeight(unsigned char b, int x, int z);
    static void GetElevation(const FunctionCallbackInfo<Value>& args);
    float getElevation(int x, int z);
    static void GetTemperature(const FunctionCallbackInfo<Value>& args);
    double getTemperature(double x, double z);
    // static void FillBiomes(const FunctionCallbackInfo<Value>& args);
    void fillBiomes(int ox, int oz, unsigned char *biomes);
    // static void FillElevations(const FunctionCallbackInfo<Value>& args);
    void fillElevations(int ox, int oz, float *elevations);
    // static void FillEther(const FunctionCallbackInfo<Value>& args);
    void fillEther(float *elevations, float *ether);
    // static void FillLiquid(const FunctionCallbackInfo<Value>& args);
    void fillLiquid(int ox, int oz, float *ether, float *elevations, float *water, float *lava);
    // static void ApplyEther(const FunctionCallbackInfo<Value>& args);
    void applyEther(float *newEther, unsigned int numNewEthers, float *ether);
    void makeGeometries(int ox, int oy, float *ether, float *water, float *lava, float *positions, unsigned int *indices, unsigned int *attributeRanges, unsigned int *indexRanges);
    static void Fill(const FunctionCallbackInfo<Value>& args);
    // static void PostProcessGeometry(const FunctionCallbackInfo<Value>& args);
    void postProcessGeometry(int ox, int oz, unsigned int *attributeRanges, float *positions, float *colors, unsigned char *biomes);
};

#endif
