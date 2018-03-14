/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

void FSEvents::emitEvent(const char *path, UInt32 flags, UInt64 id) {
  if (!handler) return;
  Napi::HandleScope handle_scope(env);
  handler.Call({
    Napi::String::New(env, path),
    Napi::Number::New(env, flags),
    Napi::Number::New(env, id)
  });
}

Napi::Value FSEvents::Stop(const Napi::CallbackInfo& info) {
  threadStop();
  asyncStop();

  return info.This();
}

Napi::Value FSEvents::Start(const Napi::CallbackInfo& info) {
  asyncStart();
  threadStart();

  return info.This();
}
