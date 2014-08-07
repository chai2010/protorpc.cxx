// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_env.h"

#if (defined(_WIN32) || defined(_WIN64))
#  include "google/protobuf/rpc/rpc_env_windows.cc"
#else
#  include "google/protobuf/rpc/rpc_env_posix.cc"
#endif

namespace google {
namespace protobuf {
namespace rpc {

void Env::Logf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  Logv(fmt, ap);
  va_end(ap);
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google
