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
#include "SkeletonSensor.h"

using namespace v8;

void DeviceConnect(uv_work_t* req) {
    DeviceBaton* baton = static_cast<DeviceBaton*>(req->data);
    
    // initialize the kinect
    baton->error_message = baton->sensor->initialize();
    // baton->sensor->setPointModeToProjective();
}

void DeviceUserConnect(uv_work_t* req) {
    DeviceBaton* baton = static_cast<DeviceBaton*>(req->data);
    
    // initialize the kinect
    baton->sensor->waitForDeviceUpdateOnUser();
}

void DeviceReady(uv_work_t* req) {
    HandleScope scope;
    DeviceBaton* baton = static_cast<DeviceBaton*>(req->data);
    
    unsigned argc = 1;
    Local<Value> argv[] = { Local<Value>::New(Undefined()) };
    
    if (baton->error_message != "") {
        Local<Value> err = Exception::Error(
                String::New(baton->error_message.c_str()));
               
        argv[0] = err;
    }
    v8::TryCatch try_catch;
        baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
        if (try_catch.HasCaught()) {
            node::FatalException(try_catch);
        }
        
    baton->callback.Dispose();
    delete baton;
}

inline Handle<Value> CHECK_RC(const unsigned int rc, const char* const description) {
    if(rc != XN_STATUS_OK) {
        return ThrowException(v8::Exception::TypeError(
            v8::String::New(xnGetStatusString(rc))));
    }

    return Null();
}

Persistent<FunctionTemplate> Scene::constructor_template;

Scene::~Scene() {
}

Scene* Scene::New() {
  HandleScope scope;

  Local<Object> scene = constructor_template->GetFunction()->NewInstance();

  return ObjectWrap::Unwrap<Scene>(scene);
}

Handle<Value> Scene::New(const Arguments &args) {
    if (!args.IsConstructCall()) {
      return node::FromConstructorTemplate(constructor_template, args);
    }
    
    HandleScope scope;
    Scene * scene = new Scene(args.This());
    
    // ev_async_init(scene->completed_, Scene::Complete);
    // ev_async_start(EV_DEFAULT_UC_ scene->completed_);
    // ev_unref(EV_DEFAULT_UC);
    
    return args.This();
}

Scene::Scene(Handle<Object> wrapper) : ObjectWrap() {
  Wrap(wrapper);
  
  sensor_ = new SkeletonSensor();
}

Handle<Value> Scene::Init(const Arguments &args) {
    HandleScope scope;
    if (args.Length() < 1) {
        return ThrowException(v8::Exception::TypeError(v8::String::New("Must provide a callback")));
    }
    
    Scene *scene = ObjectWrap::Unwrap<Scene>(args.This());

    // This creates our work request, including the libuv struct.
    DeviceBaton* baton = scene->MakeBaton(Persistent<Function>::New(Local<Function>::Cast(args[0])));
    
    int status = uv_queue_work(uv_default_loop(), &baton->request, DeviceConnect, DeviceReady);
    assert(status == 0);
  
    return Undefined();
}

Handle<Value> Scene::DetectUser(const Arguments &args) {
    HandleScope scope;
    if (args.Length() < 1) {
        return ThrowException(v8::Exception::TypeError(v8::String::New("Must provide a callback")));
    }
    
    Scene *scene = ObjectWrap::Unwrap<Scene>(args.This());

    // This creates our work request, including the libuv struct.
    DeviceBaton* baton = scene->MakeBaton(Persistent<Function>::New(Local<Function>::Cast(args[0])));

    int status = uv_queue_work(uv_default_loop(), &baton->request, DeviceUserConnect, DeviceReady);
    assert(status == 0);
  
    return Undefined();
}

DeviceBaton* Scene::MakeBaton(Persistent<Function> callback) {
    DeviceBaton *baton = new DeviceBaton();

    baton->request.data = baton;
    baton->sensor = sensor_;
    baton->callback = callback;
    
    return baton;
}

void Scene::Initialize(Handle<Object> target) {
  HandleScope scope;

  // length_symbol = Persistent<String>::New(String::NewSymbol("length"));
  // chars_written_sym = Persistent<String>::New(String::NewSymbol("_charsWritten"));

  Local<FunctionTemplate> t = FunctionTemplate::New(Scene::New);
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Scene"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "init", Scene::Init);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "detectUser", Scene::DetectUser);
  target->Set(String::NewSymbol("Scene"), constructor_template->GetFunction());
  
  scope.Close(Undefined());
}