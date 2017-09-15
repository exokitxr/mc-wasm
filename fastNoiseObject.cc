#include <node.h>
#include "fastNoiseObject.h"

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

Persistent<Function> FastNoiseObject::constructor;

FastNoiseObject::FastNoiseObject(int s, double frequency, int octaves) : fastNoise(s) {
  fastNoise.SetFrequency(frequency);
  fastNoise.SetFractalOctaves(octaves);
}

FastNoiseObject::~FastNoiseObject() {
}

double FastNoiseObject::in2D(double x, double y) {
  return (1.0 + fastNoise.GetSimplexFractal(x, y)) / 2.0;
}

void FastNoiseObject::Init(Isolate* isolate) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "FastNoiseObject"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "in2D", In2D);

  constructor.Reset(isolate, tpl->GetFunction());
}

void FastNoiseObject::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    Local<String> seedString = String::NewFromUtf8(args.GetIsolate(), "seed");
    Local<String> frequencyString = String::NewFromUtf8(args.GetIsolate(), "frequency");
    Local<String> octavesString = String::NewFromUtf8(args.GetIsolate(), "octaves");

    Local<Object> opts = args[0]->ToObject();

    int seed = opts->Get(seedString)->IntegerValue();
    double frequency = opts->Get(frequencyString)->NumberValue();
    int octaves = opts->Get(octavesString)->IntegerValue();

    FastNoiseObject* obj = new FastNoiseObject(seed, frequency, octaves);
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

void FastNoiseObject::NewInstance(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  const unsigned argc = 1;
  Local<Value> argv[argc] = {args[0]};
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

  args.GetReturnValue().Set(instance);
}

void FastNoiseObject::In2D(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  // Check the number of arguments passed.
  if (args.Length() < 2) {
    // Throw an Error that is passed back to JavaScript
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }

  // Check the argument types
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  FastNoiseObject* obj = ObjectWrap::Unwrap<FastNoiseObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->in2D(args[0]->NumberValue(), args[1]->NumberValue())));
}
