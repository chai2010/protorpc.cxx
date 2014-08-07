// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "./service.pb/arith.pb.h"
#include "./service.pb/echo.pb.h"

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

class ArithService: public service::ArithService {
 public:
  inline ArithService() {}
  virtual ~ArithService() {}

  virtual const ::google::protobuf::rpc::Error add(
    const ::service::ArithRequest* args,
    ::service::ArithResponse* reply
  ) {
    reply->set_c(args->a() + args->b());
    printf("ArithService.Add: args = a:%d b:%d, reply = c:%d, err = \"%s\"\n",
      args->a(), args->b(), reply->c(), ""
    );
    return ::google::protobuf::rpc::Error::Nil();
  }
  virtual const ::google::protobuf::rpc::Error mul(
    const ::service::ArithRequest* args,
    ::service::ArithResponse* reply
  ) {
    reply->set_c(args->a() * args->b());
    printf("ArithService.Mul: args = a:%d b:%d, reply = c:%d, err = \"%s\"\n",
      args->a(), args->b(), reply->c(), ""
    );
    return ::google::protobuf::rpc::Error::Nil();
  }
  virtual const ::google::protobuf::rpc::Error div(
    const ::service::ArithRequest* args,
    ::service::ArithResponse* reply
  ) {
    if(args->b() == 0) {
      printf("ArithService.Div: args = a:%d b:%d, reply = c:%d, err = \"%s\"\n",
        args->a(), args->b(), reply->c(), "divide by zero"
      );
      return ::google::protobuf::rpc::Error::New("divide by zero");
    }
    reply->set_c(args->a() / args->b());
    printf("ArithService.Div: args = a:%d b:%d, reply = c:%d, err = \"%s\"\n",
      args->a(), args->b(), reply->c(), ""
    );
    return ::google::protobuf::rpc::Error::Nil();
  }
  virtual const ::google::protobuf::rpc::Error error(
    const ::service::ArithRequest* args,
    ::service::ArithResponse* reply
  ) {
    printf("ArithService.Error: args = a:%d b:%d, reply = c:%d, err = \"%s\"\n",
      args->a(), args->b(), reply->c(), "ArithError"
    );
    return ::google::protobuf::rpc::Error::New("ArithError");
  }
};

class EchoService: public service::EchoService {
 public:
  inline EchoService() {}
  virtual ~EchoService() {}

  virtual const ::google::protobuf::rpc::Error Echo(
    const ::service::EchoRequest* args,
    ::service::EchoResponse* reply
  ) {
    reply->set_msg(args->msg());
    printf("EchoService.Echo: args = msg:\"%s\", reply = msg:\"%s\", err = \"%s\"\n",
      args->msg().c_str(), reply->msg().c_str(), ""
    );
    return ::google::protobuf::rpc::Error::Nil();
  }
  virtual const ::google::protobuf::rpc::Error EchoTwice(
    const ::service::EchoRequest* args,
    ::service::EchoResponse* reply
  ) {
    reply->set_msg(args->msg() + args->msg());
    printf("EchoService.EchoTwice: args = msg:\"%s\", reply = msg:\"%s\", err = \"%s\"\n",
      args->msg().c_str(), reply->msg().c_str(), ""
    );
    return ::google::protobuf::rpc::Error::Nil();
  }
};

int main() {
  ::google::protobuf::rpc::Server server;

  server.AddService(new ArithService, true);
  server.AddService(new EchoService, true);

  server.BindAndServe(1234);
  return 0;
}
