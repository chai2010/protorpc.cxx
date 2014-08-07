// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_CLIENT_H__
#define GOOGLE_PROTOBUF_RPC_CLIENT_H__

#include <google/protobuf/rpc/rpc_conn.h>
#include <google/protobuf/rpc/rpc_service.h>

namespace google {
namespace protobuf {
namespace rpc {

class Env;

class Client: public Caller {
 public:
  Client(const char* host, int port, Env* env=NULL);
  ~Client();

  const ::google::protobuf::rpc::Error CallMethod(
    const std::string& method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response);
  const ::google::protobuf::rpc::Error CallMethod(
    const ::google::protobuf::MethodDescriptor* method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response);

  // Close the connection
  void Close();

 private:
  const ::google::protobuf::rpc::Error callMethod(
    const std::string& method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response);

  bool checkMothdValid(
    const std::string& method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response) const;
  bool checkMothdValid(
    const ::google::protobuf::MethodDescriptor* method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response) const;

  std::string host_;
  int port_;
  Conn conn_;
  uint64 seq_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Client);
};

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_RPC_CLIENT_H__
