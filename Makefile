CXX=g++
CFLAGS:=-std=c++14 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Werror -pedantic -mtune=native -fno-strict-aliasing
LDFLAGS:=

.PHONY: clean

USE_BOOST:=1

ifeq ($(USE_BOOST),0)
SERVER_CC_FILES=pmp_server.cc tcp_server_epoll.cc tcp_connection_epoll.cc
SERVER_H_FILES=tcp_server.h tcp_server_epoll.h tcp_connection.h tcp_connection_epoll.h
else
SERVER_CC_FILES=pmp_server.cc tcp_server_boost.cc tcp_connection_boost.cc
SERVER_H_FILES=tcp_server.h tcp_server_boost.h tcp_connection.h tcp_connection_boost.h
CFLAGS+=-I/usr/include/boost
LDFLAGS+=-lboost_system
endif

all: pmp_server pmp_client

pmp_server: $(SERVER_CC_FILES) $(SERVER_H_FILES)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(SERVER_CC_FILES)

pmp_client: pmp_client.cc
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f pmp_server pmp_client
