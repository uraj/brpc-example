CXXFLAGS=-Wno-deprecated-declarations
BRPC_FLAGS=-Ideps/brpc/include -Ldeps/brpc/lib

CLIENT_EXTRA_FLAGS=$(BRPC_FLAGS)

all: client

compserv.pb.cc: compserv.proto
	protoc $< --cpp_out=.

compserv.pb.h: compserv.proto
	protoc $< --cpp_out=.

libcompserv.so: compserv.pb.cc compserv.pb.h
	c++ -std=c++11 -fPIC -shared $< -o $@ -lprotobuf

client: client.cpp libcompserv.so
	c++ -std=c++11 $(CXXFLAGS) $(CLIENT_EXTRA_FLAGS) -L. $< -o $@ -lbrpc -lprotobuf -lgflags -lcompserv
