/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

void FSEvents::emitEvent(const char *path, UInt32 flags, UInt64 id) {
  if (!handler) return;
  Napi::HandleScope handle_scope(env);
  Napi::Value argv[] = {
    Napi::String::New(env, path),
    Napi::Number::New(env, flags),
    Napi::Number::New(env, id)
  };
  handler->Call(3, argv);
}

Napi::Value FSEvents::New(const Napi::CallbackInfo& info) {
  std::string *path = new std::string(info[0]);
  Napi::FunctionReference *callback = new Napi::FunctionReference(info[1].As<Napi::Function>());

  FSEvents *fse = new FSEvents(**path, callback);
  fse->Wrap(info.This());

  return info.This();
}

Napi::Value FSEvents::Stop(const Napi::CallbackInfo& info) {
  FSEvents* fse = node::ObjectWrap::Unwrap<FSEvents>(info.This());

  fse->threadStop();
  fse->asyncStop();

  return info.This();
}

Napi::Value FSEvents::Start(const Napi::CallbackInfo& info) {
  FSEvents* fse = node::ObjectWrap::Unwrap<FSEvents>(info.This());
  fse->asyncStart();
  fse->threadStart();

  return info.This();
}
