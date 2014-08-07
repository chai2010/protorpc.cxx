// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_env.h"
#include "google/protobuf/stubs/once.h"

#include <queue>
#include <unistd.h>
#include <pthread.h>

namespace google {
namespace protobuf {
namespace rpc {

namespace {
struct StartThreadState {
  void (*user_function)(void*);
  void* arg;
};
}
static void* StartThreadWrapper(void* arg) {
  StartThreadState* state = reinterpret_cast<StartThreadState*>(arg);
  state->user_function(state->arg);
  delete state;
  return NULL;
}

class PosixEnv : public Env {
 public:
  PosixEnv() : page_size_(getpagesize()), started_bgthread_(false) {
    PthreadCall("mutex_init", pthread_mutex_init(&mu_, NULL));
    PthreadCall("cvar_init", pthread_cond_init(&bgsignal_, NULL));
  }
  virtual ~PosixEnv() {
    exit(1);
  }

  // Write an entry to the log file with the specified format.
  void Logv(const char* format, va_list ap) {
    const size_t kBufSize = 4096;
    char buffer[kBufSize+1];
    int written = vsnprintf(buffer, kBufSize, format, ap);
    buffer[kBufSize] = '\0';
    fprintf(stderr, "%s\n", buffer);
  }

  void StartThread(void (*function)(void* arg), void* arg) {
    pthread_t t;
    StartThreadState* state = new StartThreadState;
    state->user_function = function;
    state->arg = arg;
    PthreadCall("start thread", pthread_create(&t, NULL,  &StartThreadWrapper, state));
  }

  void Schedule(void (*function)(void*), void* arg) {
    PthreadCall("lock", pthread_mutex_lock(&mu_));

    // Start background thread if necessary
    if (!started_bgthread_) {
      started_bgthread_ = true;
      PthreadCall(
        "create thread",
        pthread_create(&bgthread_, NULL,  &PosixEnv::BGThreadWrapper, this)
      );
    }

    // If the queue is currently empty, the background thread may currently be
    // waiting.
    if(queue_.empty()) {
      PthreadCall("signal", pthread_cond_signal(&bgsignal_));
    }

    // Add to priority queue
    queue_.push_back(BGItem());
    queue_.back().function = function;
    queue_.back().arg = arg;

    PthreadCall("unlock", pthread_mutex_unlock(&mu_));
  }

 private:
  void PthreadCall(const char* label, int result) {
    if(result != 0) {
      fprintf(stderr, "pthread %s: %s\n", label, strerror(result));
      exit(1);
    }
  }

  // BGThread() is the body of the background thread
  void BGThread() {
    while (true) {
      // Wait until there is an item that is ready to run
      PthreadCall("lock", pthread_mutex_lock(&mu_));
      while (queue_.empty()) {
        PthreadCall("wait", pthread_cond_wait(&bgsignal_, &mu_));
      }

      void (*function)(void*) = queue_.front().function;
      void* arg = queue_.front().arg;
      queue_.pop_front();

      PthreadCall("unlock", pthread_mutex_unlock(&mu_));
      (*function)(arg);
    }
  }

  static void* BGThreadWrapper(void* arg) {
    reinterpret_cast<PosixEnv*>(arg)->BGThread();
    return NULL;
  }

  size_t page_size_;
  pthread_mutex_t mu_;
  pthread_cond_t bgsignal_;
  pthread_t bgthread_;
  bool started_bgthread_;

  // Entry per Schedule() call
  struct BGItem { void* arg; void (*function)(void*); };
  typedef std::deque<BGItem> BGQueue;
  BGQueue queue_;

  void* zmq_ctx_;
};

static ::google::protobuf::ProtobufOnceType g_env_default_init_once;
static Env* g_env_default;
static void InitDefaultEnv() {
  g_env_default = new PosixEnv;
}

Env* Env::Default() {
  ::google::protobuf::GoogleOnceInit(&g_env_default_init_once, InitDefaultEnv);
  return static_cast<Env *>(g_env_default);
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google
