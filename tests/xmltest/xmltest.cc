// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdio.h>

#include "./xml.pb/XmlTest.pb.h"

int main() {
  google::protobuf::xml::PbXmlTest pb1;
  google::protobuf::xml::PbXmlTest pb2;
  
  pb1.set_b(true);
  pb1.set_i(1001);
  pb1.set_f(0.123f);
  pb1.set_e(google::protobuf::xml::PB_VALUE_TYPE_INT);
  pb1.mutable_m()->set_b(false);
  pb1.mutable_m()->set_i(145);
  
  std::string xmlString;
  bool rv1 = pb1.SerializeToXmlString(&xmlString);
  printf("pb1 xml:\n%s\n", xmlString.c_str());
  
  bool b = pb2.ParseFromXmlString(xmlString);
  printf("pb2 dbg string:\n%s\n", pb2.DebugString().c_str());
  
  return 0;
}
