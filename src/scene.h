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

class Scene : public node::ObjectWrap {
  public:
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
    static v8::Persistent<v8::String> emit_symbol;

    static void Initialize(v8::Handle<v8::Object> target);
    
    ~Scene();
    
  private:
    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    static v8::Handle<v8::Value> Connect(const v8::Arguments &args);
    
    Scene();
};

#endif  // KINECT_SCENE_H_