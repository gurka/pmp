# Settings
CXX      = g++
CXXFLAGS = -std=c++14 -MMD -O2 -mtune=native \
           -Wall -Wextra \
           -pedantic -fno-strict-aliasing
LDFLAGS  = -lpthread

.PHONY: clean

# Source code
SOURCE_SERVER = src/pmp_server.cc src/mandelbrot.cc
SOURCE_CLIENT = src/pmp_client.cc src/pgm.cc
SOURCE_EPOLL  = $(wildcard src/backend_epoll/*.cc)
SOURCE_ASIO   = $(wildcard src/backend_asio/*.cc)

# Targets
all:
	@echo "Use target epoll or target asio"

epoll: bin/epoll/pmp_server bin/epoll/pmp_client

asio: CXXFLAGS += -isystem external/asio/asio/include
asio: bin/asio/pmp_server bin/asio/pmp_client

dir_guard = @mkdir -p $(@D)

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
	rm -rf bin/ obj/

-include $(addprefix obj/, $(SOURCE_SERVER:.cc=.d))
-include $(addprefix obj/, $(SOURCE_CLIENT:.cc=.d))
-include $(addprefix obj/, $(SOURCE_EPOLL:.cc=.d))
-include $(addprefix obj/, $(SOURCE_ASIO:.cc=.d))
