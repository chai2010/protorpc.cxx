// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_STUBS_DEFER_H__
#define GOOGLE_PROTOBUF_STUBS_DEFER_H__

#include <functional>

//
// Go's defer operator for C++
//
// Examples:
//
//  #include <google/protobuf/stubs/defer.h>
//
//  FILE* fp = fopen("foo.txt", "rt");
//  if(fp == NULL) return false;
//  defer([&](){ printf("fclose(fp)\n"); fclose(fp); });
//  
//  char* buf = new char[1024];
//  defer([&](){ printf("delete buf\n"); delete[] buf; });
//  
//  defer([](){ printf("defer a: %d\n", __LINE__); });
//  defer([](){ printf("defer a: %d\n", __LINE__); });
//  defer([](){ printf("defer a: %d\n", __LINE__); });
//  
//  {
//      defer([](){ printf("defer b: %d\n", __LINE__); });
//      defer([](){ printf("defer b: %d\n", __LINE__); });
//      defer([](){ printf("defer b: %d\n", __LINE__); });
//  }
//  
//  defer([](){
//      printf("defer c:\n");
//      for(int i = 0; i < 3; ++i) {
//          defer([&](){ defer([&](){
//              printf("\ti = %d: begin\n", i);
//              defer([&](){ printf("\ti = %d\n", i); });
//              printf("\ti = %d: end\n", i);
//          });});
//      }
//  });
//
// Reference:
//
// http://golang.org/doc/effective_go.html#defer
// http://golang.org/ref/spec#Defer_statements
// http://blog.korfuri.fr/post/go-defer-in-cpp/
// http://blog.korfuri.fr/attachments/go-defer-in-cpp/defer.hh
// http://blogs.msdn.com/b/vcblog/archive/2011/09/12/10209291.aspx
//

#ifndef defer
#define defer _GOOGLE_PROTOBUF_DEFER_ACTION_MAKE /* ([&](){ ... }); */
#endif

// auto _defer_action_line???_ = DeferredActionCtor([&](){ ... })
#define _GOOGLE_PROTOBUF_DEFER_ACTION_MAKE auto \
  _GOOGLE_PROTOBUF_DEFER_ACTION_VAR(_defer_action_line, __LINE__, _) = google::protobuf::DeferredActionCtor
#define _GOOGLE_PROTOBUF_DEFER_ACTION_VAR(a, b, c) _GOOGLE_PROTOBUF_DEFER_TOKEN_CONNECT(a, b, c)
#define _GOOGLE_PROTOBUF_DEFER_TOKEN_CONNECT(a, b, c) a ## b ## c

namespace google {
namespace protobuf {

// Hold defered func
class DeferredAction {
 private:
  std::function<void()> func_;

  template<typename T>
  friend DeferredAction DeferredActionCtor(T&& p);

  template<typename T>
  DeferredAction(T&& p): func_(std::bind(std::forward<T>(p))) {}

  DeferredAction();
  DeferredAction(DeferredAction const&);
  DeferredAction& operator=(DeferredAction const&);
  DeferredAction& operator=(DeferredAction&&);

 public:
  DeferredAction(DeferredAction&& other):
    func_(std::forward<std::function<void()>>(other.func_)) {
    other.func_ = nullptr;
  }
  ~DeferredAction() {
    if(func_) { func_(); }
  }
};

template<typename T>
DeferredAction DeferredActionCtor(T&& p) {
  return DeferredAction(std::forward<T>(p));
}

}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_STUBS_DEFER_H__

