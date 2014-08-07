// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_server_conn.h"
#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_wire.h>
#include <google/protobuf/stubs/defer.h>

namespace google {
namespace protobuf {
namespace rpc {

ServerConn::ServerConn(Server* server, Conn* conn, Env* env):
  server_(server), conn_(conn), env_(env) {
  //
}
ServerConn::~ServerConn() {
  conn_->Close();
  delete conn_;
}

void ServerConn::Serve(Server* server, Conn* conn, Env* env) {
  auto self = new ServerConn(server, conn, env);
  env->Schedule(ServerConn::ServeProc, self);
}

// [static]
void ServerConn::ServeProc(void* p) {
  auto self = (ServerConn*)p;
  for(;;) {
    auto err = self->ProcessOneCall(self->conn_);
    if(!err.IsNil()) {
      break;
    }
  }
  delete self;
}

Error ServerConn::ProcessOneCall(Conn* receiver) {
  wire::RequestHeader reqHeader;
  Error err;

  // 1. recv request header
  err = RecvRequestHeader(receiver, &reqHeader);
  if(!err.IsNil()) {
    return err;
  }

  // 2. find service/method
  auto method = server_->FindMethodDescriptor(reqHeader.method());
  if(method == NULL) {
    wire::SendResponse(receiver, reqHeader.id(),
      "protorpc.ServerConn.ProcessOneCall: Can't find ServiceMethod: " + reqHeader.method(),
       NULL
    );
    return Error::Nil();
  }
  auto service = server_->FindService(reqHeader.method());
  if(service == NULL) {
    wire::SendResponse(receiver, reqHeader.id(),
      "protorpc.ServerConn.ProcessOneCall: Can't find ServiceMethod: " + reqHeader.method(),
       NULL
    );
    return Error::Nil();
  }

  // 3. make request/response message
  auto request = service->GetRequestPrototype(method).New();
  auto response = service->GetResponsePrototype(method).New();
  defer([&](){ delete request; delete response; });

  // 4. recv request body
  err = wire::RecvRequestBody(receiver, &reqHeader, request);
  if(!err.IsNil()) {
    env_->Logf(
      "protorpc.ServerConn.ProcessOneCall: : RecvRequestBody fail: %s.\n",
      err.String().c_str()
    );
    return err;
  }

  // 5. call method
  auto rv = service->CallMethod(method, request, response);

  // 6. send response
  err = wire::SendResponse(receiver, reqHeader.id(), rv.String(), response);
  if(!err.IsNil()) {
    env_->Logf("protorpc.ServerConn.ProcessOneCall: : SendResponse fail: %s.\n", err.String().c_str());
    return err;
  }

  return Error::Nil();
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

