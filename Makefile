CXX=g++
CFLAGS:=-std=c++14 -Wall -Wextra -Werror -pedantic -mtune=native

.PHONY: clean

SERVER_CC_FILES=pmp_server.cc tcp_server_epoll.cc
SERVER_H_FILES=tcp_server.h tcp_server_epoll.h

all: pmp_server pmp_client

pmp_server: $(SERVER_CC_FILES) $(SERVER_H_FILES)
	$(CXX) $(CFLAGS) -o $@ $(SERVER_CC_FILES)

pmp_client: pmp_client.cc
	$(CXX) $(CFLAGS) -o $@ $^

clean:
	rm -f pmp_server pmp_client
