/*
Copyright (c) 2012, Damon Oehlman <damon.oehlman@sidelab.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef KINECT_SCENE_H_
#define KINECT_SCENE_H_

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <string>

#include <XnCppWrapper.h>
#include "SkeletonSensor.h"

class Scene : public node::ObjectWrap {
  public:
    bool active;
    xn::Context context;
    xn::UserGenerator usergen;
    xn::DepthGenerator depthgen;
    xn::ImageGenerator imagegen;
    SkeletonSensor *sensor;
    
    Skeleton users[];
    unsigned userCount;

    static v8::Persistent<v8::FunctionTemplate> constructor_template;

    static void Initialize(v8::Handle<v8::Object> target);
    
    // void Emit(int argc, v8::Handle<v8::Object> argv[]);

    ~Scene();
    
    static Scene* New();
    
  protected:
    static v8::Handle<v8::Value> Async(const v8::Arguments &args, uv_work_cb work_cb, uv_after_work_cb after_work_cb, std::string task);
    
  private:
    Scene(v8::Handle<v8::Object> wrapper);
    
    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    static v8::Handle<v8::Value> Capture(const v8::Arguments &args);
    static v8::Handle<v8::Value> CreateDepthGenerator(const v8::Arguments &args);
    static v8::Handle<v8::Value> CreateImageGenerator(const v8::Arguments &args);
    static v8::Handle<v8::Value> CreateUserGenerator(const v8::Arguments &args);
    static v8::Handle<v8::Value> StartSensor(const v8::Arguments &args);
    static v8::Handle<v8::Value> WatchForUsers(const v8::Arguments &args);
    static v8::Handle<v8::Value> InitContext(const v8::Arguments &args);
};

struct SceneBaton {
    uv_work_t request;
    v8::Persistent<v8::Function> callback;
    
    Scene *scene;
    
    std::string task;
    std::string error_message;
};

#endif  // KINECT_SCENE_H_