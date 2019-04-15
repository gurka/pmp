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

IPv4 and IPv6 support.

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

2. Build:

  2.1 Build with make:

    Build default target (asio) with make:

    $ make

    Enter output directory and run programs, example:

    $ cd bin/asio
    $ ./pmp_server 2222 &
    $ ./pmp_client -1.0 -1.0 0.0 0.0 1024 1000 1000 1 localhost:2222

    Debug build is available with DEBUG=1 flag, e.g.:

    $ make DEBUG=1

    Other targets, e.g. epoll, are also available (but currently not working):

    $ make epoll

  2.2 Build with CMake:

    Create build directory and enter it:

    $ mkdir build && cd build

    Build default target (asio):

    $ cmake ../src
    $ make

    Enter output directory and run programs, example:

    $ cd bin
    $ ./pmp_server 2222 &
    $ ./pmp_client -1.0 -1.0 0.0 0.0 1024 1000 1000 1 localhost:2222
```

### Documentation

Doxygen documentation is available [here](https://gurka.github.io/pmp/doxygen/html/index.html).

### TODO

Fully implement epoll and Winsock backend? Not really needed now since asio is both header only and cross-platform...

Still a few TODOs left in the code...

### Author
Simon Sandström

simon -at- nikanor.nu
