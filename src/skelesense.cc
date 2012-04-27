#include <v8.h>
#include <node.h>
#include <node_events.h>

#include "skelesense.h"

using namespace v8;
using namespace node;

void Scene::Init(v8::Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->Inherit(EventEmitter::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Database"));

  // NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", Open);
  // NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);
  // NODE_SET_PROTOTYPE_METHOD(constructor_template, "prepare", Prepare);
  // NODE_SET_PROTOTYPE_METHOD(constructor_template, "prepareAndStep", PrepareAndStep);

  target->Set(v8::String::NewSymbol("Scene"),
          constructor_template->GetFunction());

  // insert/update execution result mask
  // NODE_DEFINE_CONSTANT (target, EXEC_EMPTY);
  // NODE_DEFINE_CONSTANT (target, EXEC_LAST_INSERT_ID);
  // NODE_DEFINE_CONSTANT (target, EXEC_AFFECTED_ROWS);
}

Handle<Value> Scene::New(const Arguments& args) {
  HandleScope scope;
  Scene* dbo = new Scene();
  dbo->Wrap(args.This());
  return args.This();
}

Persistent<FunctionTemplate> Scene::constructor_template;