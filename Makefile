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
SOURCE_BOOST  = $(wildcard src/backend_boost/*.cc)

# Targets
all:
	@echo "Use target epoll or target boost"

epoll: bin/epoll/pmp_server bin/epoll/pmp_client

boost: CXXFLAGS += -I/usr/include/boost
boost: LDFLAGS  += -lboost_system
boost: bin/boost/pmp_server bin/boost/pmp_client

dir_guard = @mkdir -p $(@D)

obj/src/%.o: src/%.cc
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/src/backend_epoll/%.o: src/backend_epoll/%.cc
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

obj/src/backend_boost/%.o: src/backend_boost/%.cc
	$(dir_guard)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

bin/epoll/pmp_server: $(addprefix obj/, $(SOURCE_SERVER:.cc=.o)) $(addprefix obj/, $(SOURCE_EPOLL:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

bin/epoll/pmp_client: $(addprefix obj/, $(SOURCE_CLIENT:.cc=.o)) $(addprefix obj/, $(SOURCE_EPOLL:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

bin/boost/pmp_server: $(addprefix obj/, $(SOURCE_SERVER:.cc=.o)) $(addprefix obj/, $(SOURCE_BOOST:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

bin/boost/pmp_client: $(addprefix obj/, $(SOURCE_CLIENT:.cc=.o)) $(addprefix obj/, $(SOURCE_BOOST:.cc=.o))
	$(dir_guard)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -rf bin/ obj/

-include $(addprefix obj/, $(SOURCE_SERVER:.cc=.d))
-include $(addprefix obj/, $(SOURCE_CLIENT:.cc=.d))
-include $(addprefix obj/, $(SOURCE_EPOLL:.cc=.d))
-include $(addprefix obj/, $(SOURCE_BOOST:.cc=.d))
