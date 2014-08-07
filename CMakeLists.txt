# Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(PROTORPC)

#------------------------------------------------------------------------------

if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OS win64)
  else()
    set(OS win32)
  endif()
else()
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OS posix64)
  else()
    set(OS posix32)
  endif()
endif()


#------------------------------------------------------------------------------
# 3rdparty

# snappy
include_directories(AFTER
  ./3rdparty/snappy
)
set(SNAPPY_HDR
  ./3rdparty/snappy/snappy.h
  ./3rdparty/snappy/snappy-c.h
)
set(SNAPPY_SRC
  ./3rdparty/snappy/snappy.cc
  ./3rdparty/snappy/snappy-c.cc
  ./3rdparty/snappy/snappy-sinksource.cc
  ./3rdparty/snappy/snappy-stubs-internal.cc
)

# tinyxml
include_directories(AFTER
  ./3rdparty/tinyxml/include
)
set(TINYXML_HDR
  ./3rdparty/tinyxml/include/tinyxml.h
  ./3rdparty/tinyxml/include/tinystr.h
)
set(TINYXML_SRC
  ./3rdparty/tinyxml/src/tinyxml.cpp
  ./3rdparty/tinyxml/src/tinystr.cpp
  ./3rdparty/tinyxml/src/tinyxmlerror.cpp
  ./3rdparty/tinyxml/src/tinyxmlparser.cpp
)

#------------------------------------------------------------------------------
# google protobuf

if(WIN32)
  include_directories(AFTER
    ./vsprojects
    ./src
  )
else(WIN32)
  include_directories(AFTER
    ./linux
    ./src
  )
endif(WIN32)

set(PB_SRC
  ./src/google/protobuf/io/coded_stream.cc
  ./src/google/protobuf/stubs/common.cc
  ./src/google/protobuf/stubs/stringprintf.cc
  ./src/google/protobuf/descriptor.cc
  ./src/google/protobuf/descriptor.pb.cc
  ./src/google/protobuf/descriptor_database.cc
  ./src/google/protobuf/dynamic_message.cc
  ./src/google/protobuf/extension_set.cc
  ./src/google/protobuf/extension_set_heavy.cc
  ./src/google/protobuf/generated_message_reflection.cc
  ./src/google/protobuf/generated_message_util.cc
  ./src/google/protobuf/io/gzip_stream.cc
  ./src/google/protobuf/compiler/importer.cc
  ./src/google/protobuf/message.cc
  ./src/google/protobuf/message_lite.cc
  ./src/google/protobuf/stubs/once.cc
  ./src/google/protobuf/compiler/parser.cc
  ./src/google/protobuf/io/printer.cc
  ./src/google/protobuf/reflection_ops.cc
  ./src/google/protobuf/repeated_field.cc
  ./src/google/protobuf/service.cc
  ./src/google/protobuf/stubs/structurally_valid.cc
  ./src/google/protobuf/stubs/strutil.cc
  ./src/google/protobuf/stubs/substitute.cc
  ./src/google/protobuf/text_format.cc
  ./src/google/protobuf/io/tokenizer.cc
  ./src/google/protobuf/unknown_field_set.cc
  ./src/google/protobuf/wire_format.cc
  ./src/google/protobuf/wire_format_lite.cc
  ./src/google/protobuf/io/zero_copy_stream.cc
  ./src/google/protobuf/io/zero_copy_stream_impl.cc
  ./src/google/protobuf/io/zero_copy_stream_impl_lite.cc
)

set(PBC_SRC
  ./src/google/protobuf/compiler/main.cc
  ./src/google/protobuf/compiler/code_generator.cc
  ./src/google/protobuf/compiler/command_line_interface.cc
  ./src/google/protobuf/compiler/subprocess.cc
  ./src/google/protobuf/compiler/zip_writer.cc
  ./src/google/protobuf/compiler/plugin.cc
  ./src/google/protobuf/compiler/plugin.pb.cc

  ./src/google/protobuf/compiler/cpp/cpp_enum.cc
  ./src/google/protobuf/compiler/cpp/cpp_enum_field.cc
  ./src/google/protobuf/compiler/cpp/cpp_extension.cc
  ./src/google/protobuf/compiler/cpp/cpp_field.cc
  ./src/google/protobuf/compiler/cpp/cpp_file.cc
  ./src/google/protobuf/compiler/cpp/cpp_generator.cc
  ./src/google/protobuf/compiler/cpp/cpp_helpers.cc
  ./src/google/protobuf/compiler/cpp/cpp_message.cc
  ./src/google/protobuf/compiler/cpp/cpp_message_field.cc
  ./src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
  ./src/google/protobuf/compiler/cpp/cpp_service.cc
  ./src/google/protobuf/compiler/cpp/cpp_string_field.cc

  ./src/google/protobuf/compiler/java/java_enum.cc
  ./src/google/protobuf/compiler/java/java_enum_field.cc
  ./src/google/protobuf/compiler/java/java_extension.cc
  ./src/google/protobuf/compiler/java/java_field.cc
  ./src/google/protobuf/compiler/java/java_file.cc
  ./src/google/protobuf/compiler/java/java_generator.cc
  ./src/google/protobuf/compiler/java/java_helpers.cc
  ./src/google/protobuf/compiler/java/java_message.cc
  ./src/google/protobuf/compiler/java/java_message_field.cc
  ./src/google/protobuf/compiler/java/java_primitive_field.cc
  ./src/google/protobuf/compiler/java/java_service.cc
  ./src/google/protobuf/compiler/java/java_string_field.cc
  ./src/google/protobuf/compiler/java/java_doc_comment.cc

  ./src/google/protobuf/compiler/python/python_generator.cc
)

if(MSVC)
  set(PB_SRC ${PB_SRC}
    ./src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
  )
else(MSVC)
  set(PB_SRC ${PB_SRC}
    ./src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc
  )
endif(MSVC)

# ---------------------------------------------------------

# protoc --cxx_out
set(PBC_CXX24X_SRC
  ./src/google/protobuf/compiler/cxx24x/cxx_enum.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_enum_field.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_extension.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_field.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_file.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_generator.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_helpers.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_message.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_message_field.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_primitive_field.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_service.cc
  ./src/google/protobuf/compiler/cxx24x/cxx_string_field.cc
)
set(PBC_CXX25X_SRC
  ./src/google/protobuf/compiler/cxx25x/cxx_enum.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_enum_field.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_extension.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_field.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_file.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_generator.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_helpers.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_message.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_message_field.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_primitive_field.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_service.cc
  ./src/google/protobuf/compiler/cxx25x/cxx_string_field.cc
)
set(PBC_CXX_SRC
  #${PBC_CXX24X_SRC}
  ${PBC_CXX25X_SRC}
)

# xml
set(PB_XML_HDR
  ./src/google/protobuf/xml/xml_message.h
)
set(PB_XML_SRC
  ./src/google/protobuf/xml/xml_message.cc
)

# rpc
set(PB_RPC_HDR
  ./src/google/protobuf/rpc/wire.pb/wire.pb.h
  ./src/google/protobuf/rpc/rpc_service.h
  ./src/google/protobuf/rpc/rpc_server.h
  ./src/google/protobuf/rpc/rpc_server_conn.h
  ./src/google/protobuf/rpc/rpc_client.h
  ./src/google/protobuf/rpc/rpc_wire.h
  ./src/google/protobuf/rpc/rpc_conn.h

  ./src/google/protobuf/rpc/rpc_env.h
  ./src/google/protobuf/rpc/rpc_crc32.h
)
set(PB_RPC_SRC
  ./src/google/protobuf/rpc/wire.pb/wire.pb.cc
  ./src/google/protobuf/rpc/rpc_service.cc
  ./src/google/protobuf/rpc/rpc_server.cc
  ./src/google/protobuf/rpc/rpc_server_conn.cc
  ./src/google/protobuf/rpc/rpc_client.cc
  ./src/google/protobuf/rpc/rpc_wire.cc
  ./src/google/protobuf/rpc/rpc_conn.cc

  ./src/google/protobuf/rpc/rpc_env.cc
  ./src/google/protobuf/rpc/rpc_crc32.cc
)

#------------------------------------------------------------------------------

add_library(pblib STATIC
  ${PB_SRC}
  ${PB_RPC_SRC}
  ${PB_XML_SRC}

  ${SNAPPY_SRC}
  ${TINYXML_SRC}
)
if(CMAKE_BUILD_TYPE STREQUAL "debug")
  set_target_properties(pblib
    PROPERTIES OUTPUT_NAME "protorpc-${OS}-debug"
  )
else()
  set_target_properties(pblib
    PROPERTIES OUTPUT_NAME "protorpc-${OS}"
  )
endif()

add_executable(protoc
  ${PBC_SRC}
  ${PBC_CXX_SRC}

  ${PB_SRC}
)
set_target_properties(protoc
  PROPERTIES OUTPUT_NAME "protoc-${OS}"
)
if(UNIX)
  target_link_libraries(protoc pthread)
endif(UNIX)

install(TARGETS pblib protoc
  RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
  LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
  ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
)

#------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "debug")
  # xml test
  add_executable(xmltest
    ./tests/xmltest/xml.pb/XmlTest.pb.h
    ./tests/xmltest/xml.pb/XmlTest.pb.cc
    ./tests/xmltest/xmltest.cc
  )
  set_target_properties(xmltest
    PROPERTIES OUTPUT_NAME "xmltest-${OS}"
  )
  target_link_libraries(xmltest pblib)
  
  # rpc test
  add_executable(rpctest
    ./tests/rpctest/service.pb/arith.pb.h
    ./tests/rpctest/service.pb/arith.pb.cc
    ./tests/rpctest/service.pb/echo.pb.h
    ./tests/rpctest/service.pb/echo.pb.cc
    ./tests/rpctest/rpctest.cc
  )
  set_target_properties(rpctest
    PROPERTIES OUTPUT_NAME "rpctest-${OS}"
  )
  add_executable(rpcserver
    ./tests/rpctest/service.pb/arith.pb.h
    ./tests/rpctest/service.pb/arith.pb.cc
    ./tests/rpctest/service.pb/echo.pb.h
    ./tests/rpctest/service.pb/echo.pb.cc
    ./tests/rpctest/rpcserver.cc
  )
  set_target_properties(rpcserver
    PROPERTIES OUTPUT_NAME "rpcserver-${OS}"
  )
  add_executable(rpcclient
    ./tests/rpctest/service.pb/arith.pb.h
    ./tests/rpctest/service.pb/arith.pb.cc
    ./tests/rpctest/service.pb/echo.pb.h
    ./tests/rpctest/service.pb/echo.pb.cc
    ./tests/rpctest/rpcclient.cc
  )
  set_target_properties(rpcclient
    PROPERTIES OUTPUT_NAME "rpcclient-${OS}"
  )
  
  target_link_libraries(rpctest pblib)
  target_link_libraries(rpcserver pblib)
  target_link_libraries(rpcclient pblib)
  
  install(TARGETS xmltest rpctest rpcserver rpcclient
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()


#------------------------------------------------------------------------------


