#ifndef CACHED_FAST_NOISE_OBJECT_H
#define CACHED_FAST_NOISE_OBJECT_H

#include "fastNoiseObject.h"
// #include "MurmurHash3.h"
#include "hash.h"
#include <node.h>
#include <random>
#include <unordered_map>
#include <vector>

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

class CachedFastNoiseObject : public FastNoiseObject {
  public:
    static Persistent<Function> constructor;
    static void Init(Isolate* isolate);
    static void NewInstance(const FunctionCallbackInfo<Value>& args);

    std::unordered_map<std::pair<int, int>, std::vector<double>> cache;

    explicit CachedFastNoiseObject(int s, double frequency, int octaves);

    static void New(const FunctionCallbackInfo<Value>& args);
    static void In2D(const FunctionCallbackInfo<Value>& args);
    double in2D(int x, int z);
};

#endif
