/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

// constants from https://developer.apple.com/library/mac/documentation/Darwin/Reference/FSEvents_Ref/index.html#//apple_ref/doc/constant_group/FSEventStreamCreateFlags
#ifndef kFSEventStreamCreateFlagNone
#define kFSEventStreamCreateFlagNone 0x00000000
#endif

#ifndef kFSEventStreamCreateFlagUseCFTypes
#define kFSEventStreamCreateFlagUseCFTypes 0x00000001
#endif

#ifndef kFSEventStreamCreateFlagNoDefer
#define kFSEventStreamCreateFlagNoDefer 0x00000002
#endif

#ifndef kFSEventStreamCreateFlagWatchRoot
#define kFSEventStreamCreateFlagWatchRoot 0x00000004
#endif

#ifndef kFSEventStreamCreateFlagIgnoreSelf
#define kFSEventStreamCreateFlagIgnoreSelf 0x00000008
#endif

#ifndef kFSEventStreamCreateFlagFileEvents
#define kFSEventStreamCreateFlagFileEvents 0x00000010
#endif

void FSEvents::threadStart() {
  if (threadloop) {
    return;
  }

  pthread_create(&thread, nullptr, &FSEvents::threadRun, this);
}

void FSEvents::HandleStreamEvents(ConstFSEventStreamRef stream, void *ctx, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {
  FSEvents* fse = static_cast<FSEvents*>(ctx);
  
  fse->lock();
  
  for (size_t idx = 0; idx < numEvents; idx++) {
    fse->events.emplace_back(
      (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx),
      eventFlags[idx],
      eventIds[idx]);
  }
  
  fse->asyncTrigger();
  fse->unlock();
}

void *FSEvents::threadRun(void *ctx) {
  FSEvents* fse = static_cast<FSEvents*>(ctx);
  FSEventStreamContext context = { 0, ctx, nullptr, nullptr, nullptr };
  fse->threadloop = CFRunLoopGetCurrent();
  FSEventStreamRef stream = FSEventStreamCreate(nullptr, &FSEvents::HandleStreamEvents, &context, fse->paths, kFSEventStreamEventIdSinceNow, (CFAbsoluteTime) 0.1, kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes);
  FSEventStreamScheduleWithRunLoop(stream, fse->threadloop, kCFRunLoopDefaultMode);
  FSEventStreamStart(stream);
  CFRunLoopRun();
  FSEventStreamStop(stream);
  FSEventStreamUnscheduleFromRunLoop(stream, fse->threadloop, kCFRunLoopDefaultMode);
  FSEventStreamInvalidate(stream);
  FSEventStreamRelease(stream);
  fse->threadloop = nullptr;
  return nullptr;
}

void FSEvents::threadStop() {
  if (!threadloop) {
    return;
  }

  CFRunLoopStop(threadloop);
  pthread_join(thread, nullptr);
}
