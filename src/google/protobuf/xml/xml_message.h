// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GOOGLE_PROTOBUF_XML_MESSAGE_H__
#define GOOGLE_PROTOBUF_XML_MESSAGE_H__

#include <google/protobuf/message.h>

namespace google {
namespace protobuf {
namespace xml {

class XmlMessage {
 public:
  XmlMessage(Message& pbMessage);
  ~XmlMessage();

  bool LoadFile(const std::string& filename);
  bool SaveFile(const std::string& filename)const;
  
  std::string SerializeToString()const;
  bool ParseFromString(const std::string& source);
  
  std::string GetErrorText()const;

 private:
  class XmlMessageImpl* impl_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(XmlMessage);
};

}  // namespace xml
}  // namespace protobuf
}  // namespace google

#endif    // GOOGLE_PROTOBUF_XML_MESSAGE_H__

