// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_conn.h"

#if (defined(_WIN32) || defined(_WIN64))
#  include "./rpc_conn_windows.cc"
#else
#  include "./rpc_conn_posix.cc"
#endif

namespace google {
namespace protobuf {
namespace rpc {

// MaxVarintLenN is the maximum length of a varint-encoded N-bit integer.
static const int maxVarintLen16 = 3;
static const int maxVarintLen32 = 5;
static const int maxVarintLen64 = 10;

// PutUvarint encodes a uint64 into buf and returns the number of bytes written.
// If the buffer is too small, PutUvarint will panic.
static int putUvarint(uint8 buf[], uint64 x) {
  auto i = 0;
  while(x >= 0x80) {
    buf[i] = uint8(x) | 0x80;
    x >>= 7;
    i++;
  }
  buf[i] = uint8(x);
  return i + 1;
}

// ReadUvarint reads an encoded unsigned integer from r and returns it as a uint64.
bool Conn::ReadUvarint(uint64* rx) {
  uint64 x;
  uint8 s, b;

  *rx = 0;
  x = 0;
  s = 0;

  for(int i = 0; ; i++) {
    if(!Read(&b, 1)) {
      return false;
    }
    if(b < 0x80) {
      if(i > 9 || i == 9 && b > 1){
        logf("protorpc.Conn.ReadUvarint: varint overflows a 64-bit integer\n");
        return false;
      }
      *rx = (x | uint64(b)<<s);
      return true;
    }
    x |= (uint64(b&0x7f) << s);
    s += 7;
  }
  logf("not reachable!\n");
  return false;
}

// PutUvarint encodes a uint64 into buf and returns the number of bytes written.
// If the buffer is too small, PutUvarint will panic.
bool Conn::WriteUvarint(uint64 x) {
  uint8 buf[maxVarintLen64];
  int n = putUvarint(buf, x);
  return Write(buf, n);
}

// readProto reads a uvarint size and then a protobuf from r.
// If the size read is zero, nothing more is read.
bool Conn::ReadProto(::google::protobuf::Message* pb) {
  uint64 size;
  if(!ReadUvarint(&size)) {
    return false;
  }
  if(size != 0) {
    std::string buf(uint(size), '\0');
    if(!Read(&buf[0], int(size))) {
      return false;
    }
    if(!pb->ParseFromString(buf)) {
      return false;
    }
  }
  return true;
}

// writeProto writes a uvarint size and then a protobuf to w.
// If the data takes no space (like rpc.InvalidRequest),
// only a zero size is written.
bool Conn::WritePorto(const ::google::protobuf::Message* pb) {
  if(pb == NULL) {
    return WriteUvarint(uint64(0));
  }

  std::string data;
  if(!pb->SerializeToString(&data)) {
    return false;
  }
  if(!WriteUvarint(uint64(data.size()))) {
    return false;
  }
  if(!Write((void*)data.data(), data.size())) {
    return false;
  }
  return true;
}

bool Conn::RecvFrame(::std::string* data) {
  uint64 size;
  if(!ReadUvarint(&size)) {
    return false;
  }
  if(size != 0) {
    data->resize(uint(size));
    if(!Read((void*)data->data(), int(size))) {
      data->clear();
      return false;
    }
  }
  return true;
}

bool Conn::SendFrame(const ::std::string* data) {
  if(data == NULL) {
    return WriteUvarint(uint64(0));
  }

  if(!WriteUvarint(uint64(data->size()))) {
    return false;
  }
  if(!Write((void*)data->data(), data->size())) {
    return false;
  }
  return true;
}

void Conn::logf(const char* fmt, ...) {
  if(env_ != NULL) {
    va_list ap;
    va_start(ap, fmt);
    env_->Logv(fmt, ap);
    va_end(ap);
  }
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google
