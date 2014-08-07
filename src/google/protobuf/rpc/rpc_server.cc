// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_server.h"
#include "google/protobuf/rpc/rpc_env.h"

namespace google {
namespace protobuf {
namespace rpc {

Server::Server(Env* env): env_(env) {
  MutexLock locker(&mutex_);
  if(env_ == NULL) {
    env_ = Env::Default();
  }
}
Server::~Server() {
  const auto& map = service_ownership_map_;
  for(auto it = map.begin(); it != map.end(); ++it) {
    if(it->second) {
      delete service_map_[it->first];
    }
  }
}

// Add a command to the RPC server
void Server::AddService(Service* service, bool ownership) {
    auto name = Service::GetServiceName(service->GetDescriptor());
  auto it = service_map_.find(name);
  if(it != service_map_.end()) {
    GOOGLE_LOG(FATAL) << "protorpc.Server.AddService: Service already exist, " << name;
  }
  service_map_[name] = service;
  service_ownership_map_[name] = ownership;
  for(int i = 0; i < service->GetDescriptor()->method_count(); i++) {
    auto method = service->GetDescriptor()->method(i);
    auto method_name = Service::GetServiceMethodName(method);
    service_method_map_[method_name] = const_cast<::google::protobuf::MethodDescriptor*>(method);
  }
}

// Find service by method name
Service* Server::FindService(const std::string& method) {
  auto method_desc = findMethodDescriptor(method);
  if(method_desc == NULL) {
    return NULL;
  }
  auto service = findService(method_desc);
  if(service == NULL) {
    return NULL;
  }
  return service;
}

// Find method descriptor by method name
MethodDescriptor* Server::FindMethodDescriptor(const std::string& method) {
  auto method_desc = findMethodDescriptor(method);
  if(method_desc == NULL) {
    return NULL;
  }
  return method_desc;
}

void Server::BindAndServe(int port, int backlog) {
  if(!conn_.ListenTCP(port, backlog)) {
    env_->Logf("protorpc.Server.ListenTCP: fail.\n");
    exit(-1);
  }
  for(;;) {
    auto conn = conn_.Accept();
    if(conn == NULL) {
      env_->Logf("protorpc.Server.Accept: fail.\n");
      continue;
    }
    ServerConn::Serve(this, conn, env_);
  }
}

MethodDescriptor* Server::findMethodDescriptor(const std::string& method) {
  auto it = service_method_map_.find(Service::CamelCase(method));
  if(it == service_method_map_.end()) {
    return NULL;
  }
  return it->second;
}
Service* Server::findService(const ::google::protobuf::MethodDescriptor* method) {
  auto it = service_map_.find(Service::CamelCase(method->service()->name()));
  if(it == service_map_.end()) {
    return NULL;
  }
  return it->second;
}

// Call Service
const ::google::protobuf::rpc::Error Server::CallMethod(
  const std::string& method_name,
  const ::google::protobuf::Message* request,
  ::google::protobuf::Message* response
) {
  auto method = findMethodDescriptor(method_name);
  if(method == NULL) {
    return Error::New("protorpc.Server.CallMethod: can't find method " + method_name);
  }
  auto service = findService(method);
  if(service == NULL) {
    return Error::New("protorpc.Server.CallMethod: can't find service " + method_name);
  }
  return service->CallMethod(method, request, response);
}

const ::google::protobuf::rpc::Error Server::CallMethod(
  const ::google::protobuf::MethodDescriptor* method,
  const ::google::protobuf::Message* request,
  ::google::protobuf::Message* response
) {
  auto service = findService(method);
  if(service == NULL) {
        return Error::New(
          "protorpc.Server.CallMethod: can't find service " +
          Service::GetServiceMethodName(method)
        );
  }
  return service->CallMethod(method, request, response);
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

