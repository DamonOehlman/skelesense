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

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>

#include <string>
#include <iostream>
#include <XnCppWrapper.h>

using namespace node;
using namespace v8;

static Persistent<String> emit_symbol;
static Persistent<String> connect_symbol;

struct Baton {
    uv_work_t request;
    Persistent<Function> callback;
};

void Work(uv_work_t* req) {
    Baton* baton = static_cast<Baton*>(req->data);
    // SpeakThisText(baton->toSay.c_str(),strlen(baton->toSay.c_str()));
    // while(!doneFlag) {} 
}

void After(uv_work_t* req) {
    HandleScope scope;
    Baton* baton = static_cast<Baton*>(req->data);
    
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
        Local<Value>::New(Null()),
        Local<Value>::New(Null())
    };
    v8::TryCatch try_catch;
        baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
    baton->callback.Dispose();
    delete baton;
}

class NODE_EXTERN Scene : public ObjectWrap
{ 
private:
  ev_async * completed_;
  xn::Context context_;
  
public:
  static Persistent<FunctionTemplate> constructor_template;
  
  static void Init(Handle<Object> target)
  {
    HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    
    // define symbols
    connect_symbol = NODE_PSYMBOL("connect");
    emit_symbol = NODE_PSYMBOL("emit");

    constructor_template = Persistent<FunctionTemplate>::New(t);
    constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
    constructor_template->SetClassName(String::NewSymbol("Scene"));
    
    NODE_SET_PROTOTYPE_METHOD(constructor_template, "connect", Connect);
    
    target->Set(String::NewSymbol("Scene"), constructor_template->GetFunction());
  }
  
  static Handle<Value> Connect(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 1) {
        return ThrowException(v8::Exception::TypeError(v8::String::New("Must provide a callback")));
    }

    Local<Function> callback = Local<Function>::Cast(args[0]);

    // This creates our work request, including the libuv struct.
    Baton* baton = new Baton();
    baton->request.data = baton;
    // baton->toSay = toSayNonV8;
    baton->callback = Persistent<Function>::New(callback);

    int status = uv_queue_work(uv_default_loop(), &baton->request, Work, After);
    assert(status == 0);
    return Undefined();
  }
  
  Scene()
  {
    completed_ = new ev_async();
  }
  
  ~Scene()
  {
    ev_async_stop(EV_DEFAULT_UC_ completed_);
    delete completed_;
  }
  
private:
  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    Scene * scene = new Scene();
    
    // ev_async_init(scene->completed_, Scene::Complete);
    // ev_async_start(EV_DEFAULT_UC_ scene->completed_);
    // ev_unref(EV_DEFAULT_UC);
    
    scene->Wrap(args.This());
    return args.This();
  }
  
  /*
  void threadFunc()
  {
    HandleScope scope;
    sleep(100);
    Local<Value> emit_v = handle_->Get(emit_symbol);
    
    const unsigned argc = 1;
    v8::Local<v8::Value> argv[argc] = {
        v8::Local<v8::Value>::New(v8::String::New("connect"))
    };
    
    if (!emit_v->IsFunction()) return;
    Local<Function> emit = Local<Function>::Cast(emit_v);
    TryCatch tc;
    emit->Call(handle_, argc, argv);
    if (tc.HasCaught()) {
      // propagate error
    }
    
    ev_async_send(EV_DEFAULT_UC_ completed_);
  }
  */
};

extern "C" {
  void init(Handle<Object> target) {
    Scene::Init(target);
  }
  
  NODE_MODULE(skelesense, init);
}