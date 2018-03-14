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
    static void Initialize(Napi::Env env, Napi::Object exports);

    FSEvents(const Napi::CallbackInfo& info);
    ~FSEvents();

    // methods.cc - exposed
    Napi::Value Start(const Napi::CallbackInfo& info);
    Napi::Value Stop(const Napi::CallbackInfo& info);

  private:
    // locking.cc
    void lockingStart();
    void lock();
    void unlock();
    void lockingStop();

    // async.cc
    static void async_propagate(uv_async_t *async);
    void asyncStart();
    void asyncTrigger();
    void asyncStop();

    // thread.cc
    static void HandleStreamEvents(ConstFSEventStreamRef stream, void *ctx, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]);
    void threadStart();
    static void *threadRun(void *ctx);
    void threadStop();

    // methods.cc - internal
    void emitEvent(const char *path, UInt32 flags, UInt64 id);

    uv_async_t async;
    Napi::Env env;
    std::vector<fse_event> events;
    Napi::FunctionReference handler;
    pthread_mutex_t lockmutex;
    bool lockStarted;
    CFArrayRef paths;
    pthread_t thread;
    CFRunLoopRef threadloop;
  };
}

using namespace fse;

FSEvents::FSEvents(const Napi::CallbackInfo& info) :
    Napi::ObjectWrap<FSEvents>(info),
    env(info.Env()),
    handler(Napi::Persistent(info[1].As<Napi::Function>())),
    lockStarted(false),
    paths(nullptr),
    thread(nullptr),
    threadloop(nullptr) {
  std::string path(info[0].As<Napi::String>());
  CFStringRef dirs[] = { CFStringCreateWithCString(nullptr, path.c_str(), kCFStringEncodingUTF8) };
  paths = CFArrayCreate(nullptr, (const void **)&dirs, 1, nullptr);

  lockingStart();
}

FSEvents::~FSEvents() {
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
