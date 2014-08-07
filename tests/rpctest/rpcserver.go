// Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"errors"
	"log"
	"net"
	"net/rpc"

	"code.google.com/p/protorpc"
	"code.google.com/p/protorpc/service.pb"
	"code.google.com/p/goprotobuf/proto"
)

type Arith int

func (t *Arith) Add(args *service.ArithRequest, reply *service.ArithResponse) (err error) {
	reply.C = proto.Int32(args.GetA() + args.GetB())
	log.Printf("Arith.Add: args = %v, reply = %v, err = %v\n", args, reply, err)
	return
}

func (t *Arith) Mul(args *service.ArithRequest, reply *service.ArithResponse) (err error) {
	reply.C = proto.Int32(args.GetA() * args.GetB())
	log.Printf("Arith.Mul: args = %v, reply = %v, err = %v\n", args, reply, err)
	return
}

func (t *Arith) Div(args *service.ArithRequest, reply *service.ArithResponse) (err error) {
	if args.GetB() == 0 {
		err = errors.New("divide by zero")
		log.Printf("Arith.Div: args = %v, reply = %v, err = %v\n", args, reply, err)
		return
	}
	reply.C = proto.Int32(args.GetA() / args.GetB())
	log.Printf("Arith.Div: args = %v, reply = %v, err = %v\n", args, reply, err)
	return
}

func (t *Arith) Error(args *service.ArithRequest, reply *service.ArithResponse) (err error) {
	err = errors.New("ArithError")
	log.Printf("Arith.Error: args = %v, reply = %v, err = %v\n", args, reply, err)
	return
}

type Echo int

func (t *Echo) Echo(args *service.EchoRequest, reply *service.EchoResponse) (err error) {
	reply.Msg = args.Msg
	log.Printf("Echo.Echo: args = %v, reply = %v, err = %v\n", args, reply, err)
	return
}
func (t *Echo) EchoTwice(args *service.EchoRequest, reply *service.EchoResponse) (err error) {
	reply.Msg = proto.String(args.GetMsg() + args.GetMsg())
	log.Printf("Echo.Echo: args = %v, reply = %v, err = %v\n", args, reply, err)
	return
}

func main() {
	ln, err := net.Listen("tcp", "127.0.0.1:1234")
	if err != nil {
		log.Fatalln(err)
	}
	srv := rpc.NewServer()
	if err := service.RegisterArithService(srv, new(Arith)); err != nil {
		log.Fatalln(err)
	}
	if err := service.RegisterEchoService(srv, new(Echo)); err != nil {
		log.Fatalln(err)
	}
	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Fatalln(err)
		}
		go srv.ServeCodec(protorpc.NewServerCodec(conn))
	}
	panic("unreachable")
}
