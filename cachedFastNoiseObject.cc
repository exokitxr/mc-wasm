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

double CachedFastNoiseObject::in2D(int x, int z) {
  const int ox = x >> 4;
  const int oz = z >> 4;
  const int ax = x - (x & 0xFFFFFFF0);
  const int az = z - (z & 0xFFFFFFF0);
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
