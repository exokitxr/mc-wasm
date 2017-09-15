#ifndef FAST_NOISE_OBJECT_H
#define FAST_NOISE_OBJECT_H

#include <node.h>
#include <node_object_wrap.h>
#include "FastNoise.h"

class FastNoiseObject : public node::ObjectWrap {
 public:
  static v8::Persistent<v8::Function> constructor;
  static void Init(v8::Isolate* isolate);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

  FastNoise fastNoise;
  explicit FastNoiseObject(int s = 0, double frequency = 0.01, int octaves = 1);
  ~FastNoiseObject();

  double in2D(double x, double y);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void In2D(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif
