// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_CONN_H__
#define GOOGLE_PROTOBUF_RPC_CONN_H__

#include <stdarg.h>
#include <google/protobuf/message.h>

namespace google {
namespace protobuf {
namespace rpc {

class Env;

// Initialize socket services
bool InitSocket();

// Stream-oriented network connection.
class Conn {
 public:

  Conn(int fd=0, Env* env=NULL): sock_(fd), env_(env) { InitSocket(); }
  ~Conn() {}

  bool IsValid() const;
  bool DialTCP(const char* host, int port);
  bool ListenTCP(int port, int backlog=5);
  void Close();

  Conn* Accept();

  bool Read(void* buf, int len);
  bool Write(void* buf, int len);

  bool ReadUvarint(uint64* x);
  bool WriteUvarint(uint64 x);

  bool ReadProto(::google::protobuf::Message* pb);
  bool WritePorto(const ::google::protobuf::Message* pb);

  bool RecvFrame(::std::string* data);
  bool SendFrame(const ::std::string* data);

 private:
  void logf(const char* fmt, ...);

  int sock_;
  Env* env_;
};

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif // GOOGLE_PROTOBUF_RPC_CONN_H__

