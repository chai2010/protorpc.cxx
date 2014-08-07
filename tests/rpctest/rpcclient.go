// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"fmt"
	"log"
	"net/rpc"

	"code.google.com/p/protorpc"
	"code.google.com/p/protorpc/service.pb"
	"code.google.com/p/goprotobuf/proto"
)

func main() {
	var client *rpc.Client
	var err error

	client, err = protorpc.Dial("tcp", "127.0.0.1:1234")
	if err != nil {
		log.Fatal(err)
	}
	defer client.Close()

	arithStub := &service.ArithServiceClient{client}
	echoStub := &service.EchoServiceClient{client}

	var arithArgs service.ArithRequest
	var arithReply service.ArithResponse
	var echoArgs service.EchoRequest
	var echoReply service.EchoResponse

	// Add
	arithArgs.A = proto.Int32(1)
	arithArgs.B = proto.Int32(2)
	if err = arithStub.Add(&arithArgs, &arithReply); err != nil {
		log.Fatalf(`arith.Add: %v`, err)
	}
	if arithReply.GetC() != 3 {
		log.Fatalf(`arith.Add: expected = %d, got = %d`, 3, arithReply.GetC())
	}

	// Mul
	arithArgs.A = proto.Int32(2)
	arithArgs.B = proto.Int32(3)
	if err = arithStub.Mul(&arithArgs, &arithReply); err != nil {
		log.Fatalf(`arith.Mul: %v`, err)
	}
	if arithReply.GetC() != 6 {
		log.Fatalf(`arith.Mul: expected = %d, got = %d`, 6, arithReply.GetC())
	}

	// Div
	arithArgs.A = proto.Int32(13)
	arithArgs.B = proto.Int32(5)
	if err = arithStub.Div(&arithArgs, &arithReply); err != nil {
		log.Fatalf(`arith.Div: %v`, err)
	}
	if arithReply.GetC() != 2 {
		log.Fatalf(`arith.Div: expected = %d, got = %d`, 2, arithReply.GetC())
	}

	// Div zero
	arithArgs.A = proto.Int32(1)
	arithArgs.B = proto.Int32(0)
	if err = arithStub.Div(&arithArgs, &arithReply); err.Error() != "divide by zero" {
		log.Fatalf(`arith.Div: expected = "%s", got = "%s"`, "divide by zero", err.Error())
	}

	// Error
	arithArgs.A = proto.Int32(1)
	arithArgs.B = proto.Int32(2)
	if err = arithStub.Error(&arithArgs, &arithReply); err.Error() != "ArithError" {
		log.Fatalf(`arith.Error: expected = "%s", got = "%s"`, "ArithError", err.Error())
	}

	// EchoService.Echo
	echoArgs.Msg = proto.String("Hello, Protobuf-RPC")
	if err = echoStub.Echo(&echoArgs, &echoReply); err != nil {
		log.Fatalf(`echoStub.Echo: %v`, err)
	}
	if echoArgs.GetMsg() != echoReply.GetMsg() {
		log.Fatalf(`echoStub.Echo: expected = "%s", got = "%s"`, echoArgs.GetMsg(), echoReply.GetMsg())
	}

	fmt.Printf("Done\n")
}
