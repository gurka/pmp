cmake_minimum_required(VERSION 3.1)

project(pmp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 14)

if (MSVC)
  add_compile_options("/W3")
else()
  add_compile_options("-mtune=native" "-Wall" "-Wextra" "-pedantic" "-fno-strict-aliasing" "-pthread")

  # in CMake 3.14 we have add_link_options()...
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

option(USE_ASAN "Compile with ASAN (AddressSanitizer)" OFF)
if (USE_ASAN)
  add_compile_options("-fsanitize=address" "-fsanitize=leak" "-fsanitize=undefined")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
endif()

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
)
target_include_directories(backend_asio SYSTEM PUBLIC
  "../external/asio/asio/include"
)

