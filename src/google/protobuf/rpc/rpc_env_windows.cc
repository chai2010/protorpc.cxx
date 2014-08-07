// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_env.h"

#include <google/protobuf/stubs/once.h>

#include <Windows.h>
#include <WinBase.h>

#if defined(_MSC_VER)
#  pragma warning(disable:4996)
#endif

namespace google {
namespace protobuf {
namespace rpc {

namespace {
struct ThreadParam {
  ThreadParam(void (*function)(void* arg), void* arg)
    : function(function), arg(arg) {
  }

  void (*function)(void* arg);
  void* arg;
};

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
  ThreadParam * param = static_cast<ThreadParam *>(lpParameter);
  void (*function)(void* arg) = param->function;
  void* arg = param->arg;
  delete param;
  function(arg);
  return 0;
}
}  // namespace

class WindowsEnv : public Env {
 public:
  WindowsEnv() {
  }
  virtual ~WindowsEnv() {
    exit(1);
  }

  // Write an entry to the log file with the specified format.
  virtual void Logv(const char* format, va_list ap) {
    const size_t kBufSize = 4096;
    char buffer[kBufSize+1];
    int written = vsnprintf(buffer, kBufSize, format, ap);
    buffer[kBufSize] = '\0';
    fprintf(stderr, "%s\n", buffer);
  }

  // Start a new thread, invoking "function(arg)" within the new thread.
  // When "function(arg)" returns, the thread will be destroyed.
  virtual void StartThread(void (*function)(void* arg), void* arg) {
    ThreadParam * param = new ThreadParam(function, arg);
    CreateThread(NULL, 0, ThreadProc, param, 0, NULL);
  }

  // Arrange to run "(*function)(arg)" once in a background thread.
  //
  // "function" may run in an unspecified thread.  Multiple functions
  // added to the same Env may run concurrently in different threads.
  // I.e., the caller may not assume that background work items are
  // serialized.
  virtual void Schedule(void (*function)(void* arg), void* arg) {
    ThreadParam * param = new ThreadParam(function, arg);
    QueueUserWorkItem(ThreadProc, param, WT_EXECUTEDEFAULT);
  }
};

static GOOGLE_PROTOBUF_DECLARE_ONCE(g_env_default_init_once);
static Env* g_env_default;
static void InitDefaultEnv() {
  g_env_default = new WindowsEnv;
}

Env* Env::Default() {
  ::google::protobuf::GoogleOnceInit(&g_env_default_init_once, InitDefaultEnv);
  return static_cast<Env *>(g_env_default);
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google
