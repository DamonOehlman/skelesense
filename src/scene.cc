#include <node.h>
#include <node_buffer.h>

#include <v8.h>

#include <assert.h>
#include <stdlib.h> // malloc, free
#include <string.h> // memcpy

#ifdef __MINGW32__
# include "platform.h"
#endif

#ifdef __POSIX__
# include <arpa/inet.h> // htons, htonl
#endif

#include "scene.h"

using namespace v8;

Persistent<FunctionTemplate> Scene::constructor_template;

Scene::Scene() : ObjectWrap() {
}


Scene::~Scene() {
}

Handle<Value> Scene::New(const Arguments &args) {
    HandleScope scope;
    Scene * scene = new Scene();
    
    // ev_async_init(scene->completed_, Scene::Complete);
    // ev_async_start(EV_DEFAULT_UC_ scene->completed_);
    // ev_unref(EV_DEFAULT_UC);
    
    scene->Wrap(args.This());
    return args.This();
}

Handle<Value> Scene::Connect(const Arguments &args) {
  HandleScope scope;
  Scene *scene = ObjectWrap::Unwrap<Scene>(args.This());
  // SLICE_ARGS(args[0], args[1])

  /*
  char *data = parent->data_ + start;
  //Local<String> string = String::New(data, end - start);

  Local<Value> b =  Encode(data, end - start, BINARY);

  return scope.Close(b);
  */
  
  return Undefined();
}

void Scene::Initialize(Handle<Object> target) {
  HandleScope scope;

  // length_symbol = Persistent<String>::New(String::NewSymbol("length"));
  // chars_written_sym = Persistent<String>::New(String::NewSymbol("_charsWritten"));

  Local<FunctionTemplate> t = FunctionTemplate::New(Scene::New);
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Scene"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "connect", Scene::Connect);
  target->Set(String::NewSymbol("Scene"), constructor_template->GetFunction());
}