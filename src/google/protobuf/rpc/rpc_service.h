// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_RPC_SERVICE_H__
#define GOOGLE_PROTOBUF_RPC_SERVICE_H__

#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/rpc/wire.pb/wire.pb.h>

namespace google {
namespace protobuf {
namespace rpc {

// Error
class LIBPROTOBUF_EXPORT Error {
 public:
  Error(){}
  Error(const std::string& err): err_text_(err) {}
  Error(const Error& err): err_text_(err.err_text_) {}
  ~Error(){}
  
  static Error Nil() { return Error(); }
  static Error New(const std::string& err) { return Error(err); }
  
  bool IsNil()const { return err_text_.empty(); }
  const std::string& String()const { return err_text_; }

 private:
  std::string err_text_;
};

// Abstract base interface for protocol-buffer-based RPC services caller.
class LIBPROTOBUF_EXPORT Caller {
 public:
  Caller() {}
  virtual ~Caller() {}

  // Call a method of the service specified by MethodDescriptor.  This is
  // normally implemented as a simple switch() that calls the standard
  // definitions of the service's methods.
  //
  // Preconditions:
  // * method->service() == GetDescriptor()
  // * request and response are of the exact same classes as the objects
  //   returned by GetRequestPrototype(method) and
  //   GetResponsePrototype(method).
  //
  // Postconditions:
  // * "done" will be called when the method is complete.  This may be
  //   before CallMethod() returns or it may be at some point in the future.
  // * If the RPC succeeded, "response" contains the response returned by
  //   the server.
  // * If the RPC failed, "response"'s contents are undefined.
  virtual const ::google::protobuf::rpc::Error CallMethod(
    const ::google::protobuf::MethodDescriptor* method,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response) = 0;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Caller);
};

// Abstract base interface for protocol-buffer-based RPC services.  Services
// themselves are abstract interfaces (implemented either by servers or as
// stubs), but they subclass this base interface.  The methods of this
// interface can be used to call the methods of the Service without knowing
// its exact type at compile time (analogous to Reflection).
class LIBPROTOBUF_EXPORT Service: public Caller {
 public:
  Service() {}
  virtual ~Service() {}

  // CamelCase returns the CamelCased name.
  // If there is an interior underscore followed by a lower case letter,
  // drop the underscore and convert the letter to upper case.
  // There is a remote possibility of this rewrite causing a name collision,
  // but it's so remote we're prepared to pretend it's nonexistent - since the
  // C++ generator lowercases names, it's extremely unlikely to have two fields
  // with different capitalizations.
  // In short, _my_field_name_2 becomes XMyFieldName2.
  static std::string CamelCase(const std::string& s);
  
  // Get Service Name with CamelCase
  static std::string GetServiceName(const ::google::protobuf::ServiceDescriptor* service);
  // Get ServiceMethod Name with CamelCase
  // e.g. file_service.get_file_list => FileService.GetFileList
  static std::string GetServiceMethodName(const ::google::protobuf::MethodDescriptor* method);
  
  // Get the ServiceDescriptor describing this service and its methods.
  virtual const ::google::protobuf::ServiceDescriptor* GetDescriptor() = 0;
  
  // CallMethod() requires that the request and response passed in are of a
  // particular subclass of Message.  GetRequestPrototype() and
  // GetResponsePrototype() get the default instances of these required types.
  // You can then call Message::New() on these instances to construct mutable
  // objects which you can then pass to CallMethod().
  //
  // Example:
  //   const MethodDescriptor* method =
  //     service->GetDescriptor()->FindMethodByName("Foo");
  //   Message* request  = stub->GetRequestPrototype (method)->New();
  //   Message* response = stub->GetResponsePrototype(method)->New();
  //   request->ParseFromString(input);
  //   service->CallMethod(method, *request, response, callback);
  virtual const ::google::protobuf::Message& GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const = 0;
  virtual const ::google::protobuf::Message& GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const = 0;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Service);
};

}  // namespace rpc
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_RPC_SERVICE_H__