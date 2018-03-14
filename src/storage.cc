/*
 ** © 2014 by Philipp Dunkel <pip@pipobscure.com>
 ** Licensed under MIT License.
 */

struct fse_event {
  UInt64 id;
  UInt32 flags;
  CFStringRef path;
  
  fse_event(CFStringRef eventPath, UInt32 eventFlag, UInt64 eventId) :
      id(eventId),
      flags(eventFlag),
      path(eventPath) {
    
    if (path != nullptr) {
      CFRetain(path);
    }
  }
  
  ~fse_event() {
    if (path != nullptr) {
      CFRelease(path);
    }
  }

  fse_event(fse_event&& other) :
      fse_event() {
    std::swap(id, other.id);
    std::swap(flags, other.flags);
    std::swap(path, other.path);
  }

private:
  fse_event() : fse_event(nullptr, 0, 0) {}
  fse_event(const fse_event&);
  void operator=(const fse_event&);
};
