#ifndef FAST_NOISE_OBJECT_H
#define FAST_NOISE_OBJECT_H

#include <node.h>
#include <node_object_wrap.h>
#include "FastNoise.h"

class FastNoiseObject : public node::ObjectWrap {
 public:
  static void Init(v8::Isolate* isolate);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  explicit FastNoiseObject(int s = 0, double frequency = 0.01, int octaves = 1);
  ~FastNoiseObject();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void in2D(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  FastNoise fastNoise;
};

#endif
