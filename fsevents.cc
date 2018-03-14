/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "napi.h"
#include "uv.h"
#include "pthread.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include <iostream>
#include <vector>

#include "src/storage.cc"
namespace fse {
  class FSEvents : public Napi::ObjectWrap<FSEvents> {
  public:
    FSEvents(const Napi::CallbackInfo& info);
    ~FSEvents();

    // locking.cc
    bool lockStarted;
    pthread_mutex_t lockmutex;
    void lockingStart();
    void lock();
    void unlock();
    void lockingStop();

    // async.cc
    uv_async_t async;
    void asyncStart();
    void asyncTrigger();
    void asyncStop();

    // thread.cc
    pthread_t thread;
    CFRunLoopRef threadloop;
    void threadStart();
    static void *threadRun(void *ctx);
    void threadStop();

    // methods.cc - internal
    Napi::Env env;
    Napi::FunctionReference handler;
    void emitEvent(const char *path, UInt32 flags, UInt64 id);

    // Common
    CFArrayRef paths;
    std::vector<fse_event*> events;
    static void Initialize(Napi::Env env, Napi::Object exports);

    // methods.cc - exposed
    Napi::Value Start(const Napi::CallbackInfo& info);
    Napi::Value Stop(const Napi::CallbackInfo& info);

  };
}

using namespace fse;

FSEvents::FSEvents(const Napi::CallbackInfo& info) :
    Napi::ObjectWrap<FSEvents>(info),
    lockStarted(false),
    thread(NULL),
    threadloop(NULL),
    env(info.Env()),
    handler(Napi::Persistent(info[1].As<Napi::Function>())),
    paths(NULL) {
  std::string path(info[0].As<Napi::String>());
  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8) };
  paths = CFArrayCreate(NULL, (const void **)&dirs, 1, NULL);

  lockingStart();
}

FSEvents::~FSEvents() {
  std::cout << "YIKES" << std::endl;
  lockingStop();

  CFRelease(paths);
}

#ifndef kFSEventStreamEventFlagItemCreated
#define kFSEventStreamEventFlagItemCreated 0x00000010
#endif

#include "src/locking.cc"
#include "src/async.cc"
#include "src/thread.cc"
#include "src/constants.cc"
#include "src/methods.cc"

void FSEvents::Initialize(Napi::Env env, Napi::Object exports) {
  exports.Set("FSEvents", DefineClass(env, "FSEvents", {
    InstanceMethod("start", &FSEvents::Start, napi_enumerable),
    InstanceMethod("stop", &FSEvents::Stop, napi_enumerable)
  }));

  exports.Set("Constants", Constants(env));
}

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
  FSEvents::Initialize(env, exports);
  return exports;
}

NODE_API_MODULE(fse, Initialize)
