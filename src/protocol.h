#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <cstdint>
#include <complex>
#include <vector>

namespace Protocol
{

/**
 * @brief Represents a Request message
 *
 * @var  min_c        The minimum complex value
 * @var  max_c        The maximum complex value
 * @var  image_width  Image width in pixels
 * @var  image_height Image height in pixels
 * @var  max_iter     Maximum number of iterations per
 *                    sample (image pixel)
 */
struct Request
{
  std::complex<double> min_c;
  std::complex<double> max_c;
  std::uint32_t image_width;
  std::uint32_t image_height;
  std::uint32_t max_iter;
};

/**
 * @brief Represents a Response message
 *
 * @var  pixels        Array of pixels (8bpp)
 * @var  last_message  True if this is the last response message
 *                     corresponding to a request
 */
struct Response
{
  std::vector<std::uint8_t> pixels;
  bool last_message;
};

/**
 * @brief Serialize a message
 *
 * @param[in]  message  The message to serialize
 *
 * @return An array of bytes.
 */
template<typename T>
std::vector<std::uint8_t> serialize(const T& message);

/**
 * @brief Deserialize a message
 *
 * @param[in]   data     Array of bytes to deserialize
 * @param[out]  message  Pointer to message where to deserialize into
 *
 * @return  true if deserialized successfully, otherwise false
 */
template<typename T>
bool deserialize(const std::vector<std::uint8_t>& data, T* message);

}

#endif  // PROTOCOL_H_
