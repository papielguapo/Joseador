#pragma once
#include <atomic>
// Readers enter before loading the pointer. After unpublishing, the owner waits
// for readers before unmapping; an audio callback never waits or takes a lock.
template<class T> struct AudioAccess {
 std::atomic<T*> pointer{nullptr};std::atomic<unsigned> readers{0};
 struct Read {
  AudioAccess& owner;T* value;
  explicit Read(AudioAccess& o):owner(o){owner.readers.fetch_add(1);value=owner.pointer.load();}
  ~Read(){owner.readers.fetch_sub(1);}
  Read(const Read&)=delete;Read& operator=(const Read&)=delete;
 };
};
