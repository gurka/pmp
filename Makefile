CXX=g++
CFLAGS:=-std=c++14 -Wall -Wextra -Werror -pedantic -mtune=native

.PHONY: clean

all: pmp_server pmp_client

pmp_server: pmp_server.cc
	$(CXX) $(CFLAGS) -o $@ $^

pmp_client: pmp_client.cc
	$(CXX) $(CFLAGS) -o $@ $^

clean:
	rm -f pmp_server pmp_client
