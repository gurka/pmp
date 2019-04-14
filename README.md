# pmp
### Features

Cross-platform (customizable network backend):
* Cross-platform header only library [asio](https://think-async.com/)
  **Status**: fully implemented [src/backend_asio](src/backend_asio)

* Native Linux BSD sockets & epoll
  **Status**: not fully implemented [src/backend_epoll](src/backend_epoll)

* Native Windows Winsock
  **Status**: not started

Server capable of handling multiple connections in parallel, but only one Mandelbrot computation at a time.
Custom binary network protocol, see [src/protocol.h](src/protocol.h).

### Build and run

Only tested on Linux 4.9.0 and 5.0.1 with gcc 6.3.0. Requires C++14-capable compiler.

1. Clone the repository with --recursive so that the asio submodule is cloned as well:
  `git clone --recursive https://github.com/gurka/pmp.git`
  If you already cloned the repo without --recursive you can enter the repo and run:
  `git submodule update --init --recursive`

2. Enter repo and run make
  `cd pmp && make`
  This will build the default target (asio backend).

3. Enter asio build output directory and run the programs:
  `cd bin/asio`
  Example test run:
  `./pmp_server 2222 &`
  `./pmp_client -1.0 -1.0 0.0 0.0 1024 1000 1000 1 localhost:2222`

Debug builds are available with with `DEBUG=1` flag to make, e.g. `make DEBUG=1`.
You can build with epoll network backend with target epoll: `make epoll`.

### Documentation

Doxygen documentation is available [here](https://gurka.github.io/pmp/doxygen/html/index.html).

### TODO

There are a few TODOs in the code, most of them related to error handling. It should be possible for the client to continue even if connection is lost to one or more servers as long as there is at least one server still available.

Fully implement epoll and Winsock backend?

Test on Windows.

### Author
Simon Sandström
simon -at- nikanor.nu
