// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_WIRE_H__
#define GOOGLE_PROTOBUF_RPC_WIRE_H__

#include <stddef.h>
#include <stdint.h>
#include <google/protobuf/rpc/rpc_service.h>
#include <google/protobuf/rpc/rpc_conn.h>

namespace google {
namespace protobuf {
namespace rpc {
namespace wire {

Error SendRequest(Conn* conn,
  uint64_t id, const std::string& serviceMethod,
  const ::google::protobuf::Message* request
);
Error RecvRequestHeader(Conn* conn,
  RequestHeader* header
);
Error RecvRequestBody(Conn* conn,
  const RequestHeader* header,
  ::google::protobuf::Message* request
);

Error SendResponse(Conn* conn,
  uint64_t id, const std::string& error,
  const ::google::protobuf::Message* response
);
Error RecvResponseHeader(Conn* conn,
  ResponseHeader* header
);
Error RecvResponseBody(Conn* conn,
  const ResponseHeader* header,
  ::google::protobuf::Message* request
);

}  // namespace wire
}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif // GOOGLE_PROTOBUF_RPC_WIRE_H__

