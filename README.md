# pmp
### Demo

Video demo available [here](https://youtu.be/hyFlIVzvFOA).

Shows build procedure on both Windows and Linux as well as server on Windows with client on Linux and vice versa.

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

Both client and server have been tested on both Linux and Windows.

### Build and run

Requires C++14-capable compiler and make or CMake.

Tested on Linux 4.9.0 with gcc 6.3.0 and on Windows 10 with Visual Studio 2017.

```
1. Clone the repository with --recursive so that the asio submodule is cloned as well, and enter the repo:

  $ git clone --recursive https://github.com/gurka/pmp.git
  $ cd pmp

  If you already cloned the repo without --recursive you can enter the repo and run:

  $ git submodule update --init --recursive

2. Build using make or CMake:

  With make:

  $ make

  With CMake:

  $ mkdir build
  $ cd build
  $ cmake ../src
  $ make

  This will build the default target (asio backend). For CMake only the asio target is available.

3. Enter build output directory and run the programs:

  For make build:

  $ cd bin/asio

  For cmake build:

  $ cd bin

  Example test run:

  $ ./pmp_server 2222 &
  $ ./pmp_client -1.0 -1.0 0.0 0.0 1024 1000 1000 1 localhost:2222
```

Debug builds are available with with `DEBUG=1` flag to make, e.g. `make DEBUG=1`.

You can build with epoll network backend with target epoll: `make epoll`.

### Documentation

Doxygen documentation is available [here](https://gurka.github.io/pmp/doxygen/html/index.html).

### TODO

There are a few TODOs in the code, most of them related to error handling. It should be possible for the client to continue even if connection is lost to one or more servers as long as there is at least one server still available.

Fully implement epoll and Winsock backend? Not really needed now since asio is both header only and cross-platform...

### Author
Simon Sandström

simon -at- nikanor.nu
