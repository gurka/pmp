# Settings
CXX      = g++
CXXFLAGS = -std=c++14 -MMD -O2 -mtune=native \
           -Wall -Wextra \
           -pedantic -fno-strict-aliasing \
           -isystem external/protobuf/src
LDFLAGS  = -lpthread

.PHONY: clean

# Source code
PROTO_SRC = src/protocol/protocol.proto
PROTO_GEN_SRC = src/protocol/protocol.pb.cc
PROTO_GEN_INC = src/protocol/protocol.pb.h

SOURCE_SERVER    = src/pmp_server.cc $(PROTO_GEN_SRC) src/logger.cc src/mandelbrot.cc
SOURCE_CLIENT    = src/pmp_client.cc $(PROTO_GEN_SRC) src/logger.cc src/pgm.cc
SOURCE_EPOLL     = $(wildcard src/backend_epoll/*.cc)
SOURCE_ASIO      = $(wildcard src/backend_asio/*.cc)

# Targets
all: asio

epoll: bin/epoll/pmp_server bin/epoll/pmp_client

asio: CXXFLAGS += -isystem external/asio/asio/include
asio: bin/asio/pmp_server bin/asio/pmp_client

dir_guard = @mkdir -p $(@D)

$(PROTO_GEN_SRC): $(PROTO_SRC)
	protoc -I=src/protocol --cpp_out=src/protocol $^

obj/src/%.o: src/%.cc
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/src/backend_epoll/%.o: src/backend_epoll/%.cc
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

obj/src/backend_asio/%.o: src/backend_asio/%.cc
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

bin/epoll/pmp_server: $(addprefix obj/, $(SOURCE_SERVER:.cc=.o)) $(addprefix obj/, $(SOURCE_EPOLL:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

bin/epoll/pmp_client: $(addprefix obj/, $(SOURCE_CLIENT:.cc=.o)) $(addprefix obj/, $(SOURCE_EPOLL:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

bin/asio/pmp_server: $(addprefix obj/, $(SOURCE_SERVER:.cc=.o)) $(addprefix obj/, $(SOURCE_ASIO:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

bin/asio/pmp_client: $(addprefix obj/, $(SOURCE_CLIENT:.cc=.o)) $(addprefix obj/, $(SOURCE_ASIO:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -rf bin/ obj/ $(PROTO_GEN_SRC) $(PROTO_GEN_INC)

-include $(addprefix obj/, $(SOURCE_SERVER:.cc=.d))
-include $(addprefix obj/, $(SOURCE_CLIENT:.cc=.d))
-include $(addprefix obj/, $(SOURCE_EPOLL:.cc=.d))
-include $(addprefix obj/, $(SOURCE_ASIO:.cc=.d))
