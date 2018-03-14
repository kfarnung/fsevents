/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/


void FSEvents::async_propagate(uv_async_t *async) {
  if (!async->data) {
    return;
  }

  FSEvents *fse = static_cast<FSEvents*>(async->data);

  char pathbuf[1024];
  const char *pathptr = nullptr;

  fse->lock();

  for (auto const& event : fse->events) {
    pathptr = CFStringGetCStringPtr(event.path, kCFStringEncodingUTF8);
    if (!pathptr) {
      CFStringGetCString(event.path, pathbuf, 1024, kCFStringEncodingUTF8);
    }
    
    fse->emitEvent(pathptr ? pathptr : pathbuf, event.flags, event.id);
  }

  if (!fse->events.empty()) {
    fse->events.clear();
  }

  fse->unlock();
}

void FSEvents::asyncStart() {
  if (async.data == this) {
    return;
  }
  
  async.data = this;
  uv_async_init(uv_default_loop(), &async, (uv_async_cb) FSEvents::async_propagate);
}

void FSEvents::asyncTrigger() {
  if (async.data != this) {
    return;
  }
  
  uv_async_send(&async);
}

void FSEvents::asyncStop() {
  if (async.data != this) {
    return;
  }
  
  async.data = nullptr;
  uv_close((uv_handle_t *) &async, nullptr);
}
