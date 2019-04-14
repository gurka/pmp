cmake_minimum_required(VERSION 3.0)

project(pmp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -mtune=native -Wall -Wextra -pedantic -fno-strict-aliasing -pthread")

add_executable(pmp_server
  "pmp_server.cc"
  "tcp_backend.h"
  "protocol.cc"
  "protocol.h"
  "logger.cc"
  "logger.h"
  "mandelbrot.cc"
  "mandelbrot.h"
)
target_link_libraries(pmp_server
  backend_asio
)

add_executable(pmp_client
  "pmp_client.cc"
  "tcp_backend.h"
  "protocol.cc"
  "protocol.h"
  "logger.cc"
  "logger.h"
  "pgm.cc"
  "pgm.h"
)

target_link_libraries(pmp_client
  backend_asio
)

add_library(backend_asio
  "backend_asio/tcp_backend_asio.cc"
  "backend_asio/tcp_client_asio.cc"
  "backend_asio/tcp_client_asio.h"
  "backend_asio/tcp_connection_asio.cc"
  "backend_asio/tcp_connection_asio.h"
  "backend_asio/tcp_server_asio.cc"
  "backend_asio/tcp_server_asio.h"
)
target_include_directories(backend_asio PUBLIC
  "."
  "../external/asio/asio/include"
)

