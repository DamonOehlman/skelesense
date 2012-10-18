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
#include "nitools.h"
#include "SkeletonSensor.h"
#include "log.h"

static v8::Persistent<v8::String> emit_symbol = NODE_PSYMBOL("emit");

using namespace v8;

/* openni callbacks */

void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    Scene* scene = static_cast<Scene*>(pCookie);
    
    Local<Value> argv[] = { String::New("newuser"), Integer::New(nId) };
    Local<Value> emit_v = scene->handle_->Get(emit_symbol);
    
    if (emit_v->IsFunction()) {
        Local<Function> emit = Local<Function>::Cast(emit_v);

        // fire the callback
        v8::TryCatch try_catch;
        emit->Call(Context::GetCurrent()->Global(), 2, argv);
        if (try_catch.HasCaught()) {
            node::FatalException(try_catch);
        }
    }
    
    scene->usergen.GetSkeletonCap().RequestCalibration(nId, true);
}

void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
     put_flog(LOG_DEBUG, "Lost user %d", nId);
}

void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie) {
     put_flog(LOG_DEBUG, "Calibration started for user %d", nId);
}

void XN_CALLBACK_TYPE calibrationCompleteCallback(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie) {
    Scene* scene = static_cast<Scene*>(pCookie);
    
    if(eStatus == XN_CALIBRATION_STATUS_OK) {
        // put_flog(LOG_DEBUG, "Calibration completed: start tracking user %d", nId);
        scene->usergen.GetSkeletonCap().StartTracking(nId);
    }
    else {
        // put_flog(LOG_DEBUG, "Calibration failed for user %d", nId);
        scene->usergen.GetSkeletonCap().RequestCalibration(nId, true);
    }
}

/* async workers */

void capture(uv_work_t* req) {
  SceneBaton* baton = static_cast<SceneBaton*>(req->data);
  Scene* scene = baton->scene;
  SkeletonSensor* sensor = scene->sensor;
  
  sensor->waitForDeviceUpdateOnUser();
  
  // initialise the user count and the user array
  scene->userCount = sensor->getNumTrackedUsers();
  
  // resize the user data if required
  if (scene->users.size() != scene->userCount) {
      scene->users.resize(scene->userCount);
      scene->uids.resize(scene->userCount);
  }
  
  for (unsigned int ii = 0; ii < scene->userCount; ii++) {
      unsigned int uid = sensor->getUID(ii);
      
      scene->uids[ii] = uid;
      scene->users[ii] = sensor->getSkeleton(uid);
  }
  
  // initialize the kinect
  // baton->error_message = CHECK_RC(baton->scene->context.InitFromXmlFile("config/skelesense.xml"), baton->task.c_str());
}

void captureResult(uv_work_t* req) {
    HandleScope scope;
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    Scene* scene = baton->scene;
    
    unsigned int argc = scene->userCount + 1;
    Local<Value> argv[argc];
    
    // if we captured an error, then update the callback parameter
    if (baton->error_message != "") {
        Local<Value> err = Exception::Error(
                String::New(baton->error_message.c_str()));
               
        argv[0] = err;
    }
    else {
        argv[0] = Local<Value>::New(Undefined());
    }
    
    // pass through the skeleton data
    for (unsigned int ii = 0; ii < scene->userCount; ii++) {
        Persistent<Object> skel = Persistent<Object>::New(Object::New());
        Skeleton data = scene->users[ii];
        
        // set the uid
        skel->Set(String::NewSymbol("uid"), Integer::New(scene->uids[ii]));
        
        // iterate through node types and add the data to the 
        for (int nodeIdx = 0; nodeIdx < NODETYPE_COUNT; nodeIdx++) {
            // initialise the points array (x, y, z, _confidence)
            Local<Array> pointData = Local<Array>::New(Array::New());
            
            // convert the point data to am array
            pointData->Set(Integer::New(0), Integer::New(data.points[nodeIdx].x));
            pointData->Set(Integer::New(1), Integer::New(data.points[nodeIdx].y));
            pointData->Set(Integer::New(2), Integer::New(data.points[nodeIdx].z));
            pointData->Set(Integer::New(3), Integer::New(data.points[nodeIdx].confidence));
            
            skel->Set(
                String::NewSymbol(NODETYPES[nodeIdx].c_str()),
                pointData
            );
        }

        argv[ii + 1] = Local<Value>::New(skel);
    }
    
    // fire the callback
    v8::TryCatch try_catch;
        baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
        if (try_catch.HasCaught()) {
            node::FatalException(try_catch);
        }
        
    // cleanup
    baton->callback.Dispose();
    delete baton;
}

void initContext(uv_work_t* req) {
  SceneBaton* baton = static_cast<SceneBaton*>(req->data);
  Scene* scene = baton->scene;
  
  baton->error_message = scene->sensor->initialize();
  scene->sensor->setPointModeToProjective();

  // initialize the kinect
  // baton->error_message = CHECK_RC(baton->scene->context.InitFromXmlFile("config/skelesense.xml"), baton->task.c_str());
}

void createDepthGen(uv_work_t* req) {
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    Scene* scene = baton->scene;

    // initialize the kinect
    baton->error_message = CHECK_RC(scene->depthgen.Create(scene->context), baton->task.c_str());
}

void createImageGen(uv_work_t* req) {
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    Scene* scene = baton->scene;

    // initialize the kinect
    baton->error_message = CHECK_RC(scene->imagegen.Create(scene->context), baton->task.c_str());
}

void createUserGen(uv_work_t* req) {
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    Scene* scene = baton->scene;

    // initialize the kinect
    baton->error_message = CHECK_RC(scene->usergen.Create(scene->context), baton->task.c_str());
}

void startSensor(uv_work_t* req) {
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    Scene* scene = baton->scene;

    XnMapOutputMode mapMode;
    scene->depthgen.GetMapOutputMode(mapMode);

    // for now, make output map VGA resolution at 30 FPS
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS  = 30;

    scene->depthgen.SetMapOutputMode(mapMode);
    scene->imagegen.SetMapOutputMode(mapMode);
    
    // turn on device mirroring
    if(scene->depthgen.IsCapabilitySupported("Mirror") == true) {
        baton->error_message = CHECK_RC(scene->depthgen.GetMirrorCap().SetMirror(true), baton->task.c_str());
    }
    
    // turn on device mirroring
    if(scene->imagegen.IsCapabilitySupported("Mirror") == true) {
        baton->error_message = CHECK_RC(scene->imagegen.GetMirrorCap().SetMirror(true), baton->task.c_str());
    }

    // make sure the user points are reported from the POV of the depth generator
    scene->usergen.GetAlternativeViewPointCap().SetViewPoint(scene->depthgen);
    scene->depthgen.GetAlternativeViewPointCap().SetViewPoint(scene->imagegen);

    // set smoothing factor
    scene->usergen.GetSkeletonCap().SetSmoothing(0.9);

    // start data streams
    scene->context.StartGeneratingAll();
}

void watchForUsers(uv_work_t* req) {
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    Scene* scene = baton->scene;
    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete;
    
    // start data streams
    scene->context.StartGeneratingAll();

   	if (!scene->usergen) {
		//If usergen is NULL, get it from the skeleton sensor
  		scene->usergen = scene->sensor->getUserGenerator();
	}
	scene->usergen.RegisterUserCallbacks(
        newUserCallback, 
        lostUserCallback, 
        scene, 
        hUserCallbacks
    );
    
    scene->usergen.GetSkeletonCap().RegisterToCalibrationStart(
        calibrationStartCallback, 
        scene,
        hCalibrationStart
    );
    
    scene->usergen.GetSkeletonCap().RegisterToCalibrationComplete(
        calibrationCompleteCallback, 
        scene, 
        hCalibrationComplete
    );

    // turn on tracking of all joints
    scene->usergen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
}

void errorCheck(uv_work_t* req) {
    HandleScope scope;
    SceneBaton* baton = static_cast<SceneBaton*>(req->data);
    
    unsigned argc = 1;
    Local<Value> argv[] = { Local<Value>::New(Undefined()) };
    
    // if we captured an error, then update the callback parameter
    if (baton->error_message != "") {
        Local<Value> err = Exception::Error(
                String::New(baton->error_message.c_str()));
               
        argv[0] = err;
    }
    
    // fire the callback
    v8::TryCatch try_catch;
        baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
        if (try_catch.HasCaught()) {
            node::FatalException(try_catch);
        }
        
    // cleanup
    baton->callback.Dispose();
    delete baton;
}

Persistent<FunctionTemplate> Scene::constructor_template;

Scene::~Scene() {
    context.Release();
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
    Scene *scene = new Scene(args.This());
    
    return scope.Close(args.This());
}

Scene::Scene(Handle<Object> wrapper) : ObjectWrap() {
  Wrap(wrapper);
  
  sensor = new SkeletonSensor();
  active = TRUE;
}

Handle<Value> Scene::Async(const Arguments &args, uv_work_cb work_cb, uv_after_work_cb after_work_cb, std::string task) {
    HandleScope scope;
    if (args.Length() < 1) {
        return ThrowException(v8::Exception::TypeError(v8::String::New("Must provide a callback")));
    }
    
    // unwrap the scene object
    Scene *scene = ObjectWrap::Unwrap<Scene>(args.This());

    // create the baton to pass stuff around with
    SceneBaton *baton = new SceneBaton();
    baton->request.data = baton;
    baton->scene = scene;
    baton->task = task;
    baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[0]));

    // This creates our work request, including the libuv struct.
    int status = uv_queue_work(uv_default_loop(), &baton->request, work_cb, after_work_cb);
    assert(status == 0);
  
    return scope.Close(Undefined());
}

/*
void Scene::Emit(int argc, Handle<Object> argv[]) {
    HandleScope scope;
    Local<Value> emit_v = handle_->Get(emit_symbol);

    if (emit_v->IsFunction()) {
        Local<Function> emit = Local<Function>::Cast(emit_v);

        // fire the callback
        v8::TryCatch try_catch;
        emit->Call(Context::GetCurrent()->Global(), 2, argv);
        if (try_catch.HasCaught()) {
            node::FatalException(try_catch);
        }
    }
    
    scope.Close(Undefined());
}
*/

Handle<Value> Scene::Capture(const Arguments &args) {
    return Async(args, capture, captureResult, "Capturing User Input");
}

Handle<Value> Scene::CreateDepthGenerator(const Arguments &args) {
    return Async(args, createDepthGen, errorCheck, "Creating Depth Generator");
}

Handle<Value> Scene::CreateImageGenerator(const Arguments &args) {
    return Async(args, createImageGen, errorCheck, "Creating Image Generator");
}

Handle<Value> Scene::CreateUserGenerator(const Arguments &args) {
    return Async(args, createUserGen, errorCheck, "Creating User Generator");
}

Handle<Value> Scene::InitContext(const Arguments &args) {
    return Async(args, initContext, errorCheck, "Initializing Context");
}

Handle<Value> Scene::StartSensor(const Arguments &args) {
    return Async(args, startSensor, errorCheck, "Starting Sensor");
}

Handle<Value> Scene::WatchForUsers(const Arguments &args) {
    return Async(args, watchForUsers, errorCheck, "Watching for Users");
}

void Scene::Initialize(Handle<Object> target) {
  HandleScope scope;
  
  Local<FunctionTemplate> t = FunctionTemplate::New(Scene::New);
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Scene"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_init", Scene::InitContext);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_createUserGenerator", Scene::CreateUserGenerator);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_createDepthGenerator", Scene::CreateDepthGenerator);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_createImageGenerator", Scene::CreateImageGenerator);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_start", Scene::StartSensor);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_capture", Scene::Capture);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "watchForUsers", Scene::WatchForUsers);

  // NODE_SET_PROTOTYPE_METHOD(constructor_template, "detectUser", Scene::DetectUser);
  target->Set(String::NewSymbol("Scene"), constructor_template->GetFunction());

  scope.Close(Undefined());
}
