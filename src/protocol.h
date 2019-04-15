#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <cstdint>
#include <complex>
#include <vector>

namespace Protocol
{

/**
 * @brief Represents a Request message
 */
struct Request
{
  std::complex<double> min_c;  /**< The minimum complex value */
  std::complex<double> max_c;  /**< The maximum complex value */
  std::uint32_t image_width;   /**< Image width in pixels */
  std::uint32_t image_height;  /**< Image height in pixels */
  std::uint32_t max_iter;      /**< Maximum number of iterations
                                    per sample (image pixel) */
};

/**
 * @brief Represents a Response message
 */
struct Response
{
  std::vector<std::uint8_t> pixels;  /**< Array of pixels (8bpp)
                                          Note that this array may not
                                          be larger than 2^15.
                                          @see protocol.cc */
  bool last_message;                 /**< True if this is the last
                                          response message */
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
