// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "./service.pb/arith.pb.h"
#include "./service.pb/echo.pb.h"

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>

int main() {
  ::google::protobuf::rpc::Client client("127.0.0.1", 1234);

  service::ArithService::Stub arithStub(&client);
  service::EchoService::Stub echoStub(&client);

  ::service::ArithRequest arithArgs;
  ::service::ArithResponse arithReply;
  ::service::EchoRequest echoArgs;
  ::service::EchoResponse echoReply;
  ::google::protobuf::rpc::Error err;

  // EchoService.add
  arithArgs.set_a(1);
  arithArgs.set_b(2);
  err = arithStub.add(&arithArgs, &arithReply);
  if(!err.IsNil()) {
    fprintf(stderr, "arithStub.add: %s\n", err.String().c_str());
    return -1;
  }
  if(arithReply.c() != 3) {
    fprintf(stderr, "arithStub.add: expected = %d, got = %d\n", 3, arithReply.c());
    return -1;
  }

  // EchoService.mul
  arithArgs.set_a(3);
  arithArgs.set_b(4);
  err = arithStub.mul(&arithArgs, &arithReply);
  if(!err.IsNil()) {
    fprintf(stderr, "arithStub.mul: %s\n", err.String().c_str());
    return -1;
  }
  if(arithReply.c() != 12) {
    fprintf(stderr, "arithStub.mul: expected = %d, got = %d\n", 12, arithReply.c());
    return -1;
  }

  // EchoService.div
  arithArgs.set_a(13);
  arithArgs.set_b(5);
  err = arithStub.div(&arithArgs, &arithReply);
  if(!err.IsNil()) {
    fprintf(stderr, "arithStub.div: %s\n", err.String().c_str());
    return -1;
  }
  if(arithReply.c() != 2) {
    fprintf(stderr, "arithStub.div: expected = %d, got = %d\n", 2, arithReply.c());
    return -1;
  }

  // EchoService.div: divide by zero
  arithArgs.set_a(1);
  arithArgs.set_b(0);
  err = arithStub.div(&arithArgs, &arithReply);
  if(err.IsNil() || err.String() != "divide by zero") {
    fprintf(stderr, "arithStub.div: expected = %s, got = %s\n",
      "divide by zero", err.String().c_str()
    );
    return -1;
  }

  // EchoService.Echo
  echoArgs.set_msg("Hello Protobuf-RPC!");
  err = echoStub.Echo(&echoArgs, &echoReply);
  if(!err.IsNil()) {
    fprintf(stderr, "echoStub.Echo: %s\n", err.String().c_str());
    return -1;
  }
  if(echoReply.msg() != echoArgs.msg()) {
    fprintf(stderr, "echoStub.Echo: expected = \"%s\", got = \"%s\"\n",
      echoArgs.msg().c_str(), echoReply.msg().c_str()
    );
    return -1;
  }

  // EchoService.Echo: Use Client
  echoArgs.set_msg("Hello Protobuf-RPC!");
  err = client.CallMethod("EchoService.Echo", &echoArgs, &echoReply);
  if(!err.IsNil()) {
    fprintf(stderr, "EchoService.Echo: %s\n", err.String().c_str());
    return -1;
  }
  if(echoReply.msg() != echoArgs.msg()) {
    fprintf(stderr, "EchoService.Echo: expected = \"%s\", got = \"%s\"\n",
      echoArgs.msg().c_str(), echoReply.msg().c_str()
    );
    return -1;
  }

  printf("Done.\n");
  return 0;
}
