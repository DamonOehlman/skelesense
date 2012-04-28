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


/*
struct DeviceBaton {
    uv_work_t request;
    v8::Persistent<v8::Function> callback;
    
    SkeletonSensor *sensor;
    std::string error_message;
};
*/

struct ContextBaton {
    uv_work_t request;
    v8::Persistent<v8::Function> callback;
    
    xn::Context context;
    std::string error_message;
}

struct UserBaton {
    uv_work_t request;
    v8::Persistent<v8::Function> callback;
    
    xn::Context context;
    xn::UserGenerator gen;
    std::string error_message;
}

class Scene : public node::ObjectWrap {
  public:
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
    static v8::Persistent<v8::String> emit_symbol;

    static void Initialize(v8::Handle<v8::Object> target);

    ~Scene();
    
    static Scene* New();
    
  protected:
    static v8::Handle<v8::Value> WithContext(const v8::Arguments &args, uv_work_cb work_cb);
    
  private:
    // SkeletonSensor *sensor_;
    xn::Context context_;
    
    Scene(v8::Handle<v8::Object> wrapper);
    
    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    //static v8::Handle<v8::Value> Async(const v8::Arguments &args, uv_work_cb work_cb);
    //static v8::Handle<v8::Value> Init(const v8::Arguments &args);
    static v8::Handle<v8::Value> InitContext(const v8::Arguments &args);
    //static v8::Handle<v8::Value> DetectUser(const v8::Arguments &args);
};

#endif  // KINECT_SCENE_H_