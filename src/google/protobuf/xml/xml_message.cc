// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "xml_message.h"
#include "xml_base64.h"

#include <tinyxml.h>
#include <google/protobuf/descriptor.h>
#include <string>

#include <stdio.h>

#include <iostream>
#include <sstream>
#include <vector>

namespace google {
namespace protobuf {
namespace xml {

// inner impl
class XmlMessageImpl {

  private: google::protobuf::Message& pb_message_;
  private: std::string error_message_;

  //-------------------------------------------------------------------------

  public: XmlMessageImpl(::google::protobuf::Message& pbMessage): pb_message_(pbMessage){}
  public: ~XmlMessageImpl(){}

  //-------------------------------------------------------------------------
  // Base64 encoding
  //-------------------------------------------------------------------------

  private: std::string binaryToBase64(const std::string& binStr)const {
    std::vector<char> binValue;
    for(size_t i = 0; i < binStr.size(); ++i) {
      binValue.push_back(binStr.at(i));
    }
  
    std::vector<char> base64data;
    int iostatus = 0;
    Base64<char> encoder;
    std::back_insert_iterator<std::vector<char> > ins = std::back_inserter(base64data);
    encoder.put(binValue.begin(), binValue.end(), ins, iostatus, Base64<>::crlf());
  
    std::string base64Str;
    base64Str.append(base64data.begin(), base64data.end());
    return base64Str;
  }
  private: std::string binaryFromBase64(const std::string& base64Str)const {
    std::vector<char> binValue;
    int iostatus = 0;
    Base64<char> decoder;
    std::back_insert_iterator<std::vector<char> > ins = std::back_inserter(binValue);
    decoder.get(base64Str.begin(), base64Str.end(), ins, iostatus);
  
    std::string binStr;
    for(size_t i = 0; i < binValue.size(); ++i) {
      binStr.insert(binStr.end(), 1, binValue[i]);
    }
    return binStr;
  }

  //-------------------------------------------------------------------------
  // LoadFile/ParseFromString
  //-------------------------------------------------------------------------

  public: bool LoadFile(const std::string& filename) {
    // clear old data
    pb_message_.Clear();
    error_message_.clear();
  
    // load xml string
    TiXmlDocument doc;
    if(!doc.LoadFile(filename.c_str())) {
      std::stringstream buffer;
      buffer << "Xml load file fail: " << doc.ErrorDesc();
      error_message_ = buffer.str();
      return false;
    }
  
    // fetch root node
    const TiXmlNode *root = doc.FirstChild("message");
    if(!root) {
      std::stringstream buffer;
      buffer << "Xml: can not find \"message\" tag.";
      error_message_ = buffer.str();
      return false;
    }
  
    // parse message(recursive)
    return generateMessageFromXml(root, pb_message_);
  }

  public: bool ParseFromString(const std::string& xmlData) {
    // clear old data
    pb_message_.Clear();
    error_message_.clear();
  
    // load xml string
    TiXmlDocument doc;
    doc.Parse(xmlData.c_str());
    if(doc.Error()) {
      std::stringstream buffer;
      buffer << "Xml parse fail: " << doc.ErrorDesc();
      error_message_ = buffer.str();
      return false;
    }
  
    // fetch root node
    const TiXmlNode *root = doc.FirstChild("message");
    if(!root) {
      std::stringstream buffer;
      buffer << "Xml: can not find \"message\" tag.";
      error_message_ = buffer.str();
      return false;
    }
  
    // parse message(recursive)
    return generateMessageFromXml(root, pb_message_);
  }

  //-----------------------------------------------------

  private: bool generateMessageFromXml(const TiXmlNode* node, ::google::protobuf::Message& message) {
    const ::google::protobuf::Reflection* reflection = message.GetReflection();
    const ::google::protobuf::Descriptor* descriptor = message.GetDescriptor();
  
    // fetch message name
    // <message name="PbXmlTest" full_name="snt.ivms.test.PbXmlTest">
    std::string name = node->ToElement()->Attribute("name");
    std::string full_name = node->ToElement()->Attribute("full_name");
  
    if(name != descriptor->name() || full_name != descriptor->full_name()) {
      std::stringstream buffer;
      buffer  << "xml message name(\"" << full_name
          << "\") does not match pb message name(\"" << descriptor->full_name() << "\").";
      error_message_ = buffer.str();
      return false;
    }
  
    // traverse fields
    // <field label="required" type="bool" name="b" tag="1" default="true">
    for(const TiXmlNode* p = node->FirstChild("field"); p; p = p->NextSibling("field")) {
  
      // fetch field descriptor
      std::string name = p->ToElement()->Attribute("name");
      const ::google::protobuf::FieldDescriptor *field = descriptor->FindFieldByName(name);
      if(!field) {
        std::stringstream buffer;
        buffer  << "can not find field: " << name << ".";
        error_message_ = buffer.str();
        return false;
      }
  
      // fill fields value by xml
      if(!generateMessageFieldValueFromXml(p, message, reflection, field)) return false;
    }
  
    return true;
  }

  private: bool generateMessageFieldValueFromXml(
    const TiXmlNode* node, ::google::protobuf::Message& message,
    const ::google::protobuf::Reflection* reflection,
    const ::google::protobuf::FieldDescriptor* field
  ) {
    // check label
    std::string label = node->ToElement()->Attribute("label");
  
    bool bLabelMatch = true;
    if(field->is_required()) { if(label != "required") bLabelMatch = false; }
    if(field->is_optional()) { if(label != "optional") bLabelMatch = false; }
    if(field->is_repeated()) { if(label != "repeated") bLabelMatch = false; }
  
    if(!bLabelMatch) {
      std::stringstream buffer;
      buffer  << "xml field label(" << label << ") do not match pb label type.";
      error_message_ = buffer.str();
      return false;
    }
  
    switch(field->TypeToCppType(field->type())) {
    case ::google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        if(node->ToElement()->Attribute("type") != std::string("bool")) return false;
        std::vector<bool> vList = readFieldValueBool(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddBool(&message, field, vList[i]);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetBool(&message, field, vList[i]);
          }
        }
        return true;
      }
    case ::google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        if(node->ToElement()->Attribute("type") != std::string("int32")) return false;
        std::vector<google::protobuf::int32> vList = readFieldValueInt32(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddInt32(&message, field, vList[i]);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetInt32(&message, field, vList[i]);
          }
        }
        return true;
      }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        if(node->ToElement()->Attribute("type") != std::string("int64")) return false;
        std::vector<google::protobuf::int64> vList = readFieldValueInt64(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddInt64(&message, field, vList[i]);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetInt64(&message, field, vList[i]);
          }
        }
        return true;
      }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        if(node->ToElement()->Attribute("type") != std::string("uint32")) return false;
        std::vector<google::protobuf::uint32> vList = readFieldValueUInt32(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddUInt32(&message, field, vList[i]);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetUInt32(&message, field, vList[i]);
          }
        }
        return true;
      }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
        if(node->ToElement()->Attribute("type") != std::string("uint64")) return false;
        std::vector<google::protobuf::uint64> vList = readFieldValueUInt64(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddUInt64(&message, field, vList[i]);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetUInt64(&message, field, vList[i]);
          }
        }
        return true;
      }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
        if(node->ToElement()->Attribute("type") != std::string("float")) return false;
        std::vector<double> vList = readFieldValueDouble(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddFloat(&message, field, float(vList[i]));
          } else {
            if(vList.size() > 1) return false;
            reflection->SetFloat(&message, field, float(vList[i]));
          }
        }
        return true;
      }
    case ::google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        if(node->ToElement()->Attribute("type") != std::string("double")) return false;
        std::vector<double> vList = readFieldValueDouble(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          if(field->is_repeated()) {
            reflection->AddDouble(&message, field, vList[i]);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetDouble(&message, field, vList[i]);
          }
        }
        return true;
      }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        if(node->ToElement()->Attribute("type") != std::string("string")) return false;
        std::vector<std::string> vList = readFieldValueString(node, field->is_repeated());
  
        for(size_t i = 0 ; i < vList.size(); ++i) {
  
          // binary data: use base64 encoding
          // xml meta character has been encoded by TinyXml
          std::string value = vList[i];
          if(field->type() == ::google::protobuf::FieldDescriptor::TYPE_BYTES) {
            value = binaryFromBase64(value);
          }
  
          if(field->is_repeated()) {
            reflection->AddString(&message, field, value);
          } else {
            if(vList.size() > 1) return false;
            reflection->SetString(&message, field, value);
          }
        }
        return true;
      }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        std::string enumName = field->enum_type()->name();
        if(node->ToElement()->Attribute("type") != std::string(enumName)) return false;
        
        const ::google::protobuf::EnumDescriptor* enumDescriptor = field->enum_type();
        std::vector<std::string> vList = readFieldValueString(node, field->is_repeated());
        for(size_t i = 0 ; i < vList.size(); ++i) {
          const ::google::protobuf::EnumValueDescriptor* vEnum;
          vEnum = enumDescriptor->FindValueByName(vList[i]);
          if(field->is_repeated()) {
            if(vEnum) reflection->AddEnum(&message, field, vEnum);
          } else {
            if(vList.size() > 1) return false;
            if(vEnum) reflection->SetEnum(&message, field, vEnum);
          }
        }
        return true;
      }
    case ::google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        std::string msgName = field->message_type()->name();
        if(node->ToElement()->Attribute("type") != std::string(msgName)) return false;
  
        // traverse embedded message
        int index = 0;
        for(const TiXmlNode* p = node->FirstChild("message"); p; p = p->NextSibling("message")) {
          if(field->is_repeated()) {
            ::google::protobuf::Message *pSubMsg = reflection->AddMessage(&message, field);
            generateMessageFromXml(p, (*pSubMsg));
          } else {
            // if(vList.size() > 1) return false;
            ::google::protobuf::Message *pSubMsg = reflection->MutableMessage(&message, field);
            generateMessageFromXml(p, (*pSubMsg));
          }
          index++;
        }
        return true;
      }
  
    default: return false;
    }
  
    return true;
  }

  //-----------------------------------------------------

  private: std::vector<bool> readFieldValueBool(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> vs = readFieldValueData(node, bRepeated);
    std::vector<bool> v;
  
    for(size_t i = 0; i < vs.size(); ++i) {
      bool x = (vs[i] == "true")? true: false;
      v.push_back(x);
    }
    return v;
  }

  private: std::vector<google::protobuf::int32> readFieldValueInt32(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> vs = readFieldValueData(node, bRepeated);
    std::vector<google::protobuf::int32> v;
    std::stringstream tostr_buffer;
    google::protobuf::int32 t;
  
    for(size_t i = 0; i < vs.size(); ++i) {
      tostr_buffer.clear();
      tostr_buffer << vs[i];
      tostr_buffer >> t;
      v.push_back(t);
    }
    return v;
  }
  private: std::vector<google::protobuf::uint32> readFieldValueUInt32(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> vs = readFieldValueData(node, bRepeated);
    std::vector<google::protobuf::uint32> v;
    std::stringstream tostr_buffer;
    google::protobuf::uint32 t;
  
    for(size_t i = 0; i < vs.size(); ++i) {
      tostr_buffer.clear();
      tostr_buffer << std::hex << vs[i];
      tostr_buffer >> t;
      v.push_back(t);
    }
    return v;
  }
  
  private: std::vector<google::protobuf::int64> readFieldValueInt64(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> vs = readFieldValueData(node, bRepeated);
    std::vector<google::protobuf::int64> v;
    std::stringstream tostr_buffer;
    google::protobuf::int64 t;
  
    for(size_t i = 0; i < vs.size(); ++i) {
      tostr_buffer.clear();
      tostr_buffer << vs[i];
      tostr_buffer >> t;
      v.push_back(t);
    }
    return v;
  }
  private: std::vector<google::protobuf::uint64> readFieldValueUInt64(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> vs = readFieldValueData(node, bRepeated);
    std::vector<google::protobuf::uint64> v;
    std::stringstream tostr_buffer;
    google::protobuf::uint64 t;
  
    for(size_t i = 0; i < vs.size(); ++i) {
      tostr_buffer.clear();
      tostr_buffer << std::hex << vs[i];
      tostr_buffer >> t;
      v.push_back(t);
    }
    return v;
  }
 
  private: std::vector<double> readFieldValueDouble(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> vs = readFieldValueData(node, bRepeated);
    std::vector<double> v;
    
    for(size_t i = 0; i < vs.size(); ++i) {
      v.push_back(atof(vs[i].c_str()));
    }
    return v;
  }
  private: std::vector<std::string> readFieldValueString(const TiXmlNode *node, bool bRepeated) {
    return readFieldValueData(node, bRepeated);
  }
  private: std::vector<std::string> readFieldValueBytes(const TiXmlNode *node, bool bRepeated) {
    return readFieldValueData(node, bRepeated);
  }
  
  private: std::vector<std::string> readFieldValueData(const TiXmlNode *node, bool bRepeated) {
    std::vector<std::string> v;
  
    // <value>true</value>
    for(const TiXmlNode* p = node->FirstChild("value"); p; p = p->NextSibling("value")) {
      const char* text = p->ToElement()->GetText();
      if(!text || !text[0]) text = "";
      v.push_back(text);
    }
    return v;
  }

  //-------------------------------------------------------------------------
  // SaveFile/SerializeToString
  //-------------------------------------------------------------------------

  public: bool SaveFile(const std::string& filename)const {
    if(filename.empty()) return false;
  
    std::string data = SerializeToString();
    if(data.empty()) return false;
  
    FILE *fp = fopen(filename.c_str(), "wt");
    if(!fp) return false;
  
    fprintf(fp, "%s", data.c_str());
    fclose(fp);
    return true;
  }

  public: std::string SerializeToString()const {
    // xml header info
    std::stringstream xmlHeadBuffer;
    xmlHeadBuffer
      << "<!-- \n"
      << "  Generated by the google::protobuf::xml::XmlMessage. DO NOT EDIT! \n"
      << "  Source: " << pb_message_.GetDescriptor()->file()->name() << "\n"
      << "-->\n\n";
    std::string xmlHead = xmlHeadBuffer.str();
  
    // format with printer
    Printer printer;
    printer.Print(xmlHead);
    printMessage(pb_message_, printer);
  
    std::string xmlString = printer.GetString();
    return xmlString;
  }

  //-----------------------------------------------------

  class Printer {
    private: size_t initial_indent_level_;
    private: std::string indent_;
    private: std::string string_buffer_;

    public: Printer(){ SetInitialIndentLevel(0); }
    public: ~Printer(){}

    //---------------------------------------

    public: void PrintMessageHead(const ::google::protobuf::Descriptor* descriptor) {
      std::stringstream buffer;
      buffer  << indent_
          << "<message name=\"" << descriptor->name()
          << "\" full_name=\"" << descriptor->full_name()
          << "\">\n";
      string_buffer_ += buffer.str();
    }
    public: void PrintMessageTail(const ::google::protobuf::Descriptor* descriptor) {
      std::stringstream buffer;
      buffer << indent_ << "</message>\n";
    
      string_buffer_ += buffer.str();
    }

    public: void PrintFieldHead(const ::google::protobuf::FieldDescriptor* field) {
      std::string label = "???";
      if(field->is_required()) label = "required";
      if(field->is_optional()) label = "optional";
      if(field->is_repeated()) label = "repeated";
    
      std::string type = "???";
      std::string defaultValue = "";
      std::stringstream defaultValueBuffer;
    
      switch(field->TypeToCppType(field->type())) {
      case ::google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
        type = "bool";
        defaultValue = field->default_value_bool()? "true": "false";
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_INT32:
        type = "int32";
        defaultValueBuffer << field->default_value_int32();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_INT64:
        type = "int64";
        defaultValueBuffer << field->default_value_int64();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
        type = "uint32";
        defaultValueBuffer << std::showbase << std::hex << field->default_value_uint32();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
        type = "uint64";
        defaultValueBuffer << std::showbase << std::hex << field->default_value_uint64();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
        type = "float";
        defaultValueBuffer << field->default_value_float();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
        type = "double";
        defaultValueBuffer << field->default_value_double();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_STRING:
        type = "string";
        defaultValueBuffer << field->default_value_string();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
        type = field->enum_type()->name();
        defaultValueBuffer << field->default_value_enum()->name();
        defaultValue = defaultValueBuffer.str();
        break;
      case ::google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
        type = field->message_type()->name();
        break;
    
      default: break;
      }
    
      // has default value?
      defaultValue = field->has_default_value()? defaultValue: std::string("");
    
      // filed tag
      std::stringstream fieldHeadBuffer;
      fieldHeadBuffer << indent_
        << "<field label=\"" << label
        << "\" type=\"" << type
        << "\" name=\"" << field->name()
        << "\" tag=\"" << field->number()
        << "\" default=\"" << defaultValue
        << "\">\n";
      string_buffer_ += fieldHeadBuffer.str();
    }
    public: void PrintFieldTail(const ::google::protobuf::FieldDescriptor* field) {
      std::stringstream buffer;
      buffer << indent_ << "</field>\n";
      string_buffer_ += buffer.str();
    }
    public: void PrintEnumDefine(const ::google::protobuf::EnumDescriptor *enumDescriptor) {
      std::stringstream bufHead;
      bufHead << indent_
        << "<enum name=\"" << enumDescriptor->name()
        << "\" full_name=\"" << enumDescriptor->full_name()
        << "\">\n";
      string_buffer_ += bufHead.str();
    
      this->Indent();
      {
        for(int i = 0; i < enumDescriptor->value_count(); ++i) {
          const ::google::protobuf::EnumValueDescriptor *vDesc = enumDescriptor->value(i);
    
          std::stringstream bufValue;
          bufValue << indent_ << "<" << vDesc->name() << ">" << vDesc->number() << "</" << vDesc->name() << ">\n";
          string_buffer_ += bufValue.str();
        }
      }
      this->Outdent();
    
      std::stringstream bufTail;
      bufTail << indent_ << "</enum>\n";
      string_buffer_ += bufTail.str();
    }

    public: void PrintValue(const std::string& value) {
      std::stringstream buffer;
      buffer << indent_ << value;
      string_buffer_ += buffer.str();
    }
    
    public: void Print(const std::string& str) {
      string_buffer_ += str;
    }

    public: std::string GetString()const{ return string_buffer_; }

    //---------------------------------------

    public: void Indent() { indent_ += "  "; }
    public: void Outdent() {
      if(!indent_.empty() && indent_.size() >= initial_indent_level_ * 2) {
        indent_.resize(indent_.size() - 2);
      }
    }
    public: void SetInitialIndentLevel(size_t indent_level) {
      initial_indent_level_ = indent_level;
    }
  };

  //-----------------------------------------------------

  private: void printMessage(const ::google::protobuf::Message& message, Printer& printer)const {
    const ::google::protobuf::Descriptor* descriptor = message.GetDescriptor();
    const ::google::protobuf::Reflection* reflection = message.GetReflection();
  
    printer.PrintMessageHead(descriptor);
    printer.Indent();
  
    for(int i = 0; i < descriptor->field_count(); i++) {
      const ::google::protobuf::FieldDescriptor *field = descriptor->field(i);
      printField(message, reflection, field, printer);
    }
  
    printer.Outdent();
    printer.PrintMessageTail(descriptor);
  }

  private: void printField(
    const ::google::protobuf::Message& message,
    const ::google::protobuf::Reflection* reflection,
    const ::google::protobuf::FieldDescriptor* field,
    Printer& printer
  )const {
    printer.PrintFieldHead(field);
    printer.Indent();
  
    int count = 0;
    if (field->is_repeated()) {
      count = reflection->FieldSize(message, field);
    } else if (reflection->HasField(message, field)) {
      count = 1;
    }
  
    for (int j = 0; j < count; ++j) {
      // Write the field value.
      int field_index = field->is_repeated()? j: -1;
      printFieldValue(message, reflection, field, field_index, printer);
    }
  
    printer.Outdent();
    printer.PrintFieldTail(field);
  }

  // if no value, no print
  private: void printFieldValue(
    const ::google::protobuf::Message& message,
    const ::google::protobuf::Reflection* reflection,
    const ::google::protobuf::FieldDescriptor* field,
    int index, Printer& printer
  )const {
    GOOGLE_DCHECK(field->is_repeated() || (index == -1))
      << "Index must be -1 for non-repeated fields";
  
    std::stringstream tostr_buffer;
    switch (field->cpp_type()) {
    case ::google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
      if (field->is_repeated()) {
        std::string value = reflection->GetRepeatedBool(message, field, index)? "true": "false";
        tostr_buffer << "<value>" << value << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        std::string value = reflection->GetBool(message, field)? "true": "false";
        tostr_buffer << "<value>" << value << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }

    case ::google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
      if (field->is_repeated()) {
        tostr_buffer << "<value>"
          << reflection->GetRepeatedInt32(message, field, index)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        tostr_buffer << "<value>"
          << reflection->GetInt32(message, field)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }
    case ::google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
      if (field->is_repeated()) {
        tostr_buffer << "<value>"
          << reflection->GetRepeatedInt64(message, field, index)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        tostr_buffer << "<value>"
          << reflection->GetInt64(message, field)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }

    case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
      if (field->is_repeated()) {
        tostr_buffer << "<value>"
          << std::showbase << std::hex << reflection->GetRepeatedUInt32(message, field, index)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        tostr_buffer << "<value>"
          << std::showbase << std::hex << reflection->GetUInt32(message, field)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }
    case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
      if (field->is_repeated()) {
        tostr_buffer << "<value>"
          << std::showbase << std::hex << reflection->GetRepeatedUInt64(message, field, index)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        tostr_buffer << "<value>"
          << std::showbase << std::hex << reflection->GetUInt64(message, field)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }

    case ::google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
      if (field->is_repeated()) {
        tostr_buffer << "<value>"
          << reflection->GetRepeatedFloat(message, field, index)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        tostr_buffer << "<value>"
          << reflection->GetFloat(message, field)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }
    case ::google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
      if (field->is_repeated()) {
        tostr_buffer << "<value>"
          << reflection->GetRepeatedDouble(message, field, index)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      } else {
        tostr_buffer << "<value>"
          << reflection->GetDouble(message, field)
          << "</value>\n";
        printer.PrintValue(tostr_buffer.str());
      }
      break;
    }
    
    case ::google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
      std::string scratch;
      std::string value = field->is_repeated() ?
                reflection->GetRepeatedStringReference(message, field, index, &scratch) :
                reflection->GetStringReference(message, field, &scratch);
  
      if(field->type() == ::google::protobuf::FieldDescriptor::TYPE_BYTES) {
        // binary: base64 encoding
        printer.PrintValue("<!-- binary: base64 encoder -->\n");
        value = binaryToBase64(value);
      } else {
        // xml meta character:
        //
        // < <--> &lt;
        // > <--> &gt;
        // & <--> &amp;
        // ' <--> &apos;
        // " <--> &quot;
        //
        // <message>if salary < 1000 then</message>
        // <message>if salary &lt; 1000 then</message>
        //
        TIXML_STRING strIn(value.c_str()), strEncoded;
        TiXmlBase::EncodeString(strIn, &strEncoded);
        value = std::string(strEncoded.c_str());
      }
  
      tostr_buffer << "<value>" << value << "</value>\n";
      printer.PrintValue(tostr_buffer.str());
      break;
    }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
      std::string value = field->is_repeated() ?
                reflection->GetRepeatedEnum(message, field, index)->name().c_str() :
                reflection->GetEnum(message, field)->name().c_str();
      tostr_buffer << "<value>" << value << "</value>\n";
      printer.PrintValue(tostr_buffer.str());
      break;
    }
  
    case ::google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
  
      int count = 0;
      if (field->is_repeated()) {
        count = reflection->FieldSize(message, field);
      } else if (reflection->HasField(message, field)) {
        count = 1;
      }
  
      // if no value, no print
      if(count > 0) {
        printMessage(field->is_repeated() ?
              reflection->GetRepeatedMessage(message, field, index) :
              reflection->GetMessage(message, field),
              printer);
      }
      break;
    }
    } // end switch
  }

  //-------------------------------------------------------------------------

  public: std::string GetErrorText()const {
    return error_message_;
  }

  //-------------------------------------------------------------------------
};

//=============================================================================

XmlMessage::XmlMessage(google::protobuf::Message& pbMessage) {
  impl_ = new XmlMessageImpl(pbMessage);
}
XmlMessage::~XmlMessage() {
  delete impl_;
}

//---------------------------------------------------------

bool XmlMessage::LoadFile(const std::string& filename) {
  return impl_->LoadFile(filename);
}
bool XmlMessage::SaveFile(const std::string& filename)const {
  return impl_->SaveFile(filename);
}

std::string XmlMessage::SerializeToString()const {
  return impl_->SerializeToString();
}
bool XmlMessage::ParseFromString(const std::string& source) {
  return impl_->ParseFromString(source);
}

std::string XmlMessage::GetErrorText()const{
  return impl_->GetErrorText();
}

}  // namespace xml
}  // namespace protobuf
}  // namespace google
