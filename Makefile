CXX=g++
CFLAGS:=-std=c++14 -Wall -Wextra -Werror -pedantic -mtune=native

.PHONY: clean

all: server client

server: server.cc
	$(CXX) $(CFLAGS) -o $@ $^

client: client.cc
	$(CXX) $(CFLAGS) -o $@ $^

clean:
	rm -f server client
