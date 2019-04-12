#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <cstdint>
#include <complex>
#include <vector>

namespace Protocol
{

struct Request
{
  std::complex<double> min_c;
  std::complex<double> max_c;
  std::uint32_t image_width;
  std::uint32_t image_height;
  std::uint32_t max_iter;
};

struct Response
{
  std::vector<std::uint8_t> pixels;
  bool last_message;
};

template<typename T>
std::vector<std::uint8_t> serialize(const T& message);

template<typename T>
bool deserialize(const std::vector<std::uint8_t>& data, T* message);

}

#endif  // PROTOCOL_H_
