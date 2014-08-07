// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_SERVER_H__
#define GOOGLE_PROTOBUF_RPC_SERVER_H__

#include <google/protobuf/rpc/rpc_service.h>
#include <google/protobuf/rpc/rpc_server_conn.h>
#include <map>

namespace google {
namespace protobuf {
namespace rpc {

class Server: public Caller {
 public:
  Server(Env* env=NULL);
  ~Server();

  // Add a command to the RPC server
  void AddService(Service* service, bool ownership);

  // Find service by method name
  Service* FindService(const std::string& method);
  // Find method descriptor by method name
  MethodDescriptor* FindMethodDescriptor(const std::string& method);

  // [blocking]
  // Process client requests for the specified time
  void BindAndServe(int port, int backlog=5);

  // Call Service Method
  const ::google::protobuf::rpc::Error CallMethod(
    const std::string& method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response
  );
  const ::google::protobuf::rpc::Error CallMethod(
    const ::google::protobuf::MethodDescriptor* method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response
  );

 private:
  MethodDescriptor* findMethodDescriptor(const std::string& method);
  Service* findService(const ::google::protobuf::MethodDescriptor* method);

  std::map<std::string, Service*> service_map_;
  std::map<std::string, bool> service_ownership_map_;
  std::map<std::string, ::google::protobuf::MethodDescriptor*> service_method_map_;

  Mutex mutex_;
  Conn conn_;
  Env* env_;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Server);
};

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_RPC_SERVER_H__

