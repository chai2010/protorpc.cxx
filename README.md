Protobuf-RPC for C++
====================

Talks: [Go/C++语言Protobuf-RPC简介](http://go-talks.appspot.com/github.com/chai2010/talks/chai2010-protorpc-intro.slide)


# Install

1. Install CMake and VS2010(Windows)/gcc(Unix)
2. cd `${protorpc_root}/protobuf` and build with cmake

# Add xml support (with `--cxx_out` option):

New API for message:

	// Parse a protocol buffer contained in a string.
	bool ParseFromXmlString(const string& data);
	// Like ParseFromXmlString(), but accepts messages that are missing
	// required fields.
	bool ParsePartialFromXmlString(const string& data);
	
	// Serialize the message and store it in the given string.  All required
	// fields must be set.
	bool SerializeToXmlString(string* output) const;
	// Like SerializeToXmlString(), but allows missing required fields.
	bool SerializePartialToXmlString(string* output) const;

Here is the simple code(`${protorpc_root}/protobuf/src/google/protobuf/xml/example/xmltest.cc`):

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

# Implement `Protobuf-RPC` (with `--cxx_out` option):

C++ use the same protocol as Go, so we can communication with C++ and Go with `Protobuf-RPC`.

Here is a simple proto file(`${protorpc_root}/protobuf/src/google/protobuf/rpc/service.pb`):

	package service;
	
	option cc_generic_services = true;
	option java_generic_services = true;
	option py_generic_services = true;
	
	message EchoRequest {
		optional string msg = 1;
	}
	
	message EchoResponse {
		optional string msg = 1;
	}
	
	service EchoService {
		rpc echo (EchoRequest) returns (EchoResponse);
	}
	
	service EchoServiceA {
		rpc echo (EchoRequest) returns (EchoResponse);
	}
	
	service EchoServiceB {
		rpc echo (EchoRequest) returns (EchoResponse);
	}

Then use new `protoc` with `--cxx_out` option to generate "arith.pb.{h|cc}" file(include rpc stub):

	${protorpc_root}/protobuf/bin/protoc --cxx_out=. echo.proto

The server calls (for TCP service):

	#include "echo.pb.h"
	
	class EchoService: public service::EchoService {
	public:
		inline EchoService() {}
		virtual ~EchoService() {}
	
		virtual const google::protobuf::rpc::Error Echo(
			const service::EchoRequest* request,
			service::EchoResponse* response
		) {
			response->set_msg(request->msg());
			return google::protobuf::rpc::Error::Nil();
		}
	};
	
	int main() {
		google::protobuf::rpc::Server server;
	
		server.AddService(new EchoService, true);
	
		server.Serve(9527);
	}

At this point, clients can see a service `EchoService` with methods `EchoService.Echo`. To invoke one, a client first dials the server:

	#include <google/protobuf/rpc/rpc_server.h>
	#include <google/protobuf/rpc/rpc_client.h>

	#include "arith.pb.h"

	int main() {
		google::protobuf::rpc::Client client("127.0.0.1", 9527);
		
		service::EchoService::Stub echoStub(&client);
	
		service::EchoRequest echoArgs;
		service::EchoResponse echoReply;
		google::protobuf::rpc::Error err;
	
		// EchoService.echo
		echoArgs.set_msg("Hello Protobuf-RPC!");
		err = echoStub.echo(&echoArgs, &echoReply);
		if(!err.IsNil()) {
			fprintf(stderr, "echoStub.echo: %s\n", err.String().c_str());
			return -1;
		}
		if(echoReply.msg() != echoArgs.msg()) {
			fprintf(stderr, "echoStub.echo: expected = \"%s\", got = \"%s\"\n",
				echoArgs.msg().c_str(), echoReply.msg().c_str()
			);
			return -1;
		}
	
		// EchoService.echo: Use Client
		echoArgs.set_msg("Hello Protobuf-RPC!");
		err = client.CallMethod("EchoService.Echo", &echoArgs, &echoReply);
		if(!err.IsNil()) {
			fprintf(stderr, "EchoService.echo: %s\n", err.String().c_str());
			return -1;
		}
		if(echoReply.msg() != echoArgs.msg()) {
			fprintf(stderr, "EchoService.echo: expected = \"%s\", got = \"%s\"\n",
				echoArgs.msg().c_str(), echoReply.msg().c_str()
			);
			return -1;
		}
	
		return 0;
	}

# BUGS

Please report bugs to <chaishushan@gmail.com>.

Thanks!
