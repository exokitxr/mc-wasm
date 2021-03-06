#include <node.h>
#include "noiseObject.h"
#include "v8-strings.h"

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

Persistent<Function> NoiseObject::constructor;

NoiseObject::NoiseObject(int s, double frequency, int octaves) : noise(s, frequency, octaves) {}

NoiseObject::~NoiseObject() {}

double NoiseObject::in2D(double x, double y) {
  return noise.in2D(x, y);
}

double NoiseObject::in3D(double x, double y, double z) {
  return noise.in3D(x, y, z);
}

void NoiseObject::Init(Isolate* isolate) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(V8_STRINGS::NoiseObject.Get(isolate));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "in2D", In2D);
  NODE_SET_PROTOTYPE_METHOD(tpl, "in3D", In3D);

  constructor.Reset(isolate, tpl->GetFunction());
}

void NoiseObject::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    Local<Object> opts = args[0]->ToObject();

    int seed = opts->Get(V8_STRINGS::seed.Get(isolate))->IntegerValue();
    double frequency = opts->Get(V8_STRINGS::frequency.Get(isolate))->NumberValue();
    int octaves = opts->Get(V8_STRINGS::octaves.Get(isolate))->IntegerValue();

    NoiseObject* obj = new NoiseObject(seed, frequency, octaves);
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

void NoiseObject::NewInstance(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  const unsigned argc = 1;
  Local<Value> argv[argc] = {args[0]};
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();

  args.GetReturnValue().Set(instance);
}

void NoiseObject::In2D(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  NoiseObject* obj = ObjectWrap::Unwrap<NoiseObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->in2D(args[0]->NumberValue(), args[1]->NumberValue())));
}

void NoiseObject::In3D(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongNumberOfArguments.Get(isolate)));
    return;
  }
  if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(V8_STRINGS::wrongArguments.Get(isolate)));
    return;
  }

  NoiseObject* obj = ObjectWrap::Unwrap<NoiseObject>(args.Holder());
  args.GetReturnValue().Set(Number::New(isolate, obj->in3D(args[0]->NumberValue(), args[1]->NumberValue(), args[2]->NumberValue())));
}
