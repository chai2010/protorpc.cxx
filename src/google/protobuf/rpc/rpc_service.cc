// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "google/protobuf/rpc/rpc_service.h"

#include <google/protobuf/descriptor.h>

namespace google {
namespace protobuf {
namespace rpc {

// [static]
// See: goprotobuf/protoc-gen-go/generator/generator.go#CamelCase
std::string Service::CamelCase(const std::string& s) {
  if(s.empty()) {
    return s;
  }
  auto t = std::string("");
  auto i = int(0);
  if(s[0] == '_') {
    // Need a capital letter; drop the '_'.
    t.append(1u, 'X');
    i++;
  }
  // Invariant: if the next letter is lower case, it must be converted
  // to upper case.
  // That is, we process a word at a time, where words are marked by _ or
  // upper case letter. Digits are treated as words.
  for( ; i < s.size(); i++) {
    auto c = s[i];
    if(c == '_' && i+1 < s.size() && islower(s[i+1])) {
      continue; // Skip the underscore in s.
    }
    if(isdigit(c) || c == '.') {
      t.append(1u, c);
      continue;
    }
    // Assume we have a letter now - if not, it's a bogus identifier.
    // The next word is a sequence of characters that must start upper case.
    if(islower(c)) {
      c ^= ' '; // Make it a capital letter.
    }
    t.append(1u, c); // Guaranteed not lower case.
    // Accept lower case sequence that follows.
    while(i+1 < s.size() && islower(s[i+1])) {
      i++;
      t.append(1u, s[i]);
    }
  }
  return t;
}

// [static]
// Get Service Name with CamelCase
std::string Service::GetServiceName(const ::google::protobuf::ServiceDescriptor* service) {
  return CamelCase(service->name());
}

// [static]
// Get ServiceMethod Name with CamelCase
// e.g. file_service.get_file_list => FileService.GetFileList
std::string Service::GetServiceMethodName(const ::google::protobuf::MethodDescriptor* method) {
  return CamelCase(method->service()->name()) + "." + CamelCase(method->name());
}

}  // namespace rpc
}  // namespace protobuf
}  // namespace google
