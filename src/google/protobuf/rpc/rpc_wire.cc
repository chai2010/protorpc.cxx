// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <google/protobuf/rpc/rpc_wire.h>
#include <google/protobuf/rpc/rpc_crc32.h>

#include <snappy.h>

namespace google {
namespace protobuf {
namespace rpc {
namespace wire {

Error SendRequest(Conn* conn,
  uint64_t id, const std::string& serviceMethod,
  const ::google::protobuf::Message* request
) {
  // marshal request
  std::string pbRequest;
  if(request != NULL) {
    if(!request->SerializeToString(&pbRequest)) {
      return Error::New("protorpc.SendRequest: SerializeToString failed.");
    }
  }

  // compress serialized proto data
  std::string compressedPbRequest;
  snappy::Compress(pbRequest.data(), pbRequest.size(), &compressedPbRequest);

  // generate header
  RequestHeader header;

  header.set_id(id);
  header.set_method(serviceMethod);

  header.set_raw_request_len(pbRequest.size());
  header.set_snappy_compressed_request_len(compressedPbRequest.size());
  header.set_checksum(HashCRC32(compressedPbRequest.data(), compressedPbRequest.size()));

  // check header size
  std::string pbHeader;
  if(!header.SerializeToString(&pbHeader)) {
    return Error::New("protorpc.SendRequest: SerializeToString failed.");
  }
  if(pbHeader.size() > Const::default_instance().max_header_len()) {
    return Error::New("protorpc.SendRequest: header larger than max_header_len.");
  }

  // send header
  if(!conn->SendFrame(&pbHeader)) {
    return Error::New("protorpc.SendRequest: SendFrame header failed.");
  }

  // send body
  if(!conn->SendFrame(&compressedPbRequest)) {
    return Error::New("protorpc.SendRequest: SendFrame body failed.");
  }

  return Error::Nil();
}

Error RecvRequestHeader(Conn* conn,
  RequestHeader* header
) {
  // recv header
  std::string pbHeader;
  if(!conn->RecvFrame(&pbHeader)) {
    return Error::New("protorpc.RecvRequestHeader: RecvFrame failed.");
  }
  if(pbHeader.size() > Const::default_instance().max_header_len()) {
    return Error::New("protorpc.RecvRequestHeader: RecvFrame larger than max_header_len.");
  }

  // Marshal Header
  if(!header->ParseFromString(pbHeader)) {
    return Error::New("protorpc.RecvRequestHeader: ParseFromString failed.");
  }

  return Error::Nil();
}

Error RecvRequestBody(Conn* conn,
  const RequestHeader* header,
  ::google::protobuf::Message* request
) {
  // recv body
  std::string compressedPbRequest;
  if(!conn->RecvFrame(&compressedPbRequest)) {
    return Error::New("protorpc.RecvRequestBody: RecvFrame failed.");
  }

  // checksum
  uint32_t checksum = HashCRC32(compressedPbRequest.data(), compressedPbRequest.size());
  if(checksum != header->checksum()) {
    return Error::New("protorpc.RecvRequestBody: Unexpected checksum.");
  }

  // decode the compressed data
  std::string pbRequest;
  if(!snappy::Uncompress(compressedPbRequest.data(), compressedPbRequest.size(), &pbRequest)) {
    return Error::New("protorpc.RecvRequestBody: snappy::Uncompress failed.");
  }
  // check wire header: rawMsgLen
  if(pbRequest.size() != header->raw_request_len()) {
    return Error::New("protorpc.RecvRequestBody: Unexcpeted raw msg len.");
  }

  // marshal request
  if(!request->ParseFromString(pbRequest)) {
    return Error::New("protorpc.RecvRequestBody: ParseFromString failed.");
  }

  return Error::Nil();
}

Error SendResponse(Conn* conn,
  uint64_t id, const std::string& error,
  const ::google::protobuf::Message* response
) {
  // marshal response
  std::string pbResponse;
  if(response != NULL) {
    if(!response->SerializeToString(&pbResponse)) {
      return Error::New("protorpc.SendResponse: SerializeToString failed.");
    }
  }

  // compress serialized proto data
  std::string compressedPbResponse;
  snappy::Compress(pbResponse.data(), pbResponse.size(), &compressedPbResponse);

  // generate header
  ResponseHeader header;

  header.set_id(id);
  header.set_error(error);

  header.set_raw_response_len(pbResponse.size());
  header.set_snappy_compressed_response_len(compressedPbResponse.size());
  header.set_checksum(HashCRC32(compressedPbResponse.data(), compressedPbResponse.size()));

  // check header size
  std::string pbHeader;
  if(!header.SerializeToString(&pbHeader)) {
    return Error::New("protorpc.SendResponse: SerializeToString failed.");
  }
  if(pbHeader.size() > Const::default_instance().max_header_len()) {
    return Error::New("protorpc.SendResponse: header larger than max_header_len.");
  }

  // send header
  if(!conn->SendFrame(&pbHeader)) {
    return Error::New("protorpc.SendResponse: SendFrame header failed.");
  }

  // send body
  if(!conn->SendFrame(&compressedPbResponse)) {
    return Error::New("protorpc.SendResponse: SendFrame body failed.");
  }

  return Error::Nil();
}

Error RecvResponseHeader(Conn* conn,
  ResponseHeader* header
) {
  // recv header
  std::string pbHeader;
  if(!conn->RecvFrame(&pbHeader)) {
    return Error::New("protorpc.RecvResponseHeader: RecvFrame failed.");
  }
  if(pbHeader.size() > Const::default_instance().max_header_len()) {
    return Error::New("protorpc.RecvResponseHeader: RecvFrame larger than max_header_len.");
  }

  // Marshal Header
  if(!header->ParseFromString(pbHeader)) {
    return Error::New("protorpc.RecvResponseHeader: ParseFromString failed.");
  }

  return Error::Nil();
}

Error RecvResponseBody(Conn* conn,
  const ResponseHeader* header,
  ::google::protobuf::Message* response
) {
  // recv body
  std::string compressedPbRequest;
  if(!conn->RecvFrame(&compressedPbRequest)) {
    return Error::New("protorpc.RecvResponseBody: RecvFrame failed.");
  }

  // checksum
  uint32_t checksum = HashCRC32(compressedPbRequest.data(), compressedPbRequest.size());
  if(checksum != header->checksum()) {
    return Error::New("protorpc.RecvResponseBody: Unexpected checksum.");
  }

  // decode the compressed data
  std::string pbResponse;
  if(!snappy::Uncompress(compressedPbRequest.data(), compressedPbRequest.size(), &pbResponse)) {
    return Error::New("protorpc.RecvResponseBody: snappy::Uncompress failed.");
  }
  // check wire header: rawMsgLen
  if(pbResponse.size() != header->raw_response_len()) {
    return Error::New("protorpc.RecvResponseBody: Unexcpeted raw msg len.");
  }

  // marshal response
  if(!response->ParseFromString(pbResponse)) {
    return Error::New("protorpc.RecvResponseBody: ParseFromString failed.");
  }

  return Error::Nil();
}

}  // namespace wire
}  // namespace rpc
}  // namespace protobuf
}  // namespace google
