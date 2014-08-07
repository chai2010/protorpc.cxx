// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_SERVER_CONN_H__
#define GOOGLE_PROTOBUF_RPC_SERVER_CONN_H__

#include <google/protobuf/rpc/rpc_env.h>
#include <google/protobuf/rpc/rpc_conn.h>
#include <google/protobuf/rpc/rpc_service.h>

namespace google {
namespace protobuf {
namespace rpc {

class Server;

class ServerConn {
 public:
  static void Serve(Server* server, Conn* conn, Env* env);

 private:
  ServerConn(Server* server, Conn* conn, Env* env);
  ~ServerConn();

  static void ServeProc(void* p);
  Error ProcessOneCall(Conn* receiver);

  const ::google::protobuf::rpc::Error callMethod(
    const std::string& method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response);

  Server* server_;
  Conn* conn_;
  Env* env_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ServerConn);
};

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_RPC_SERVER_CONN_H__

