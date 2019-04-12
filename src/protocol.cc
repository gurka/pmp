#include "protocol.h"

#include <cstdlib>
#include <iterator>

#include "logger.h"

namespace
{
  // Helpers for adding or getting raw values from a buffer

  template<typename T>
  void add(std::vector<std::uint8_t>* buffer, const T& val);

  template<>
  void add(std::vector<std::uint8_t>* buffer, const std::uint8_t& val)
  {
    buffer->push_back(val);
  }

  template<>
  void add(std::vector<std::uint8_t>* buffer, const std::uint16_t& val)
  {
    const auto* tmp = reinterpret_cast<const std::uint8_t*>(&val);
    std::copy(tmp, tmp + sizeof(val), std::back_inserter(*buffer));
  }

  template<>
  void add(std::vector<std::uint8_t>* buffer, const std::uint32_t& val)
  {
    const auto* tmp = reinterpret_cast<const std::uint8_t*>(&val);
    std::copy(tmp, tmp + sizeof(val), std::back_inserter(*buffer));
  }

  template<>
  void add(std::vector<std::uint8_t>* buffer, const bool& val)
  {
    // bool is simply a std::uint8_t where 0 = false, 1 = true
    add<std::uint8_t>(buffer, val ? 1u : 0u);
  }

  template<>
  void add(std::vector<std::uint8_t>* buffer, const double& val)
  {
    // The C++-standard does not enforce IEEE-754, so this might
    // break if the server or client is built on a strange system.
    // IEEE-754 specifies double to be 64-bits, so we can at least verify that.
    static_assert(sizeof(double) == 8, "double is not 8 bytes");
    const auto* tmp = reinterpret_cast<const std::uint8_t*>(&val);
    std::copy(tmp, tmp + sizeof(val), std::back_inserter(*buffer));
  }

  template<>
  void add(std::vector<std::uint8_t>* buffer, const std::complex<double>& val)
  {
    // std::complex<double> is simply encoded as:
    // 8 bytes: real value
    // 8 bytes: imag value
    add(buffer, val.real());
    add(buffer, val.imag());
  }

  template<>
  void add(std::vector<std::uint8_t>* buffer, const std::vector<std::uint8_t>& val)
  {
    // Byte array is encoded as:
    // 2 bytes: number of bytes
    // n bytes: bytes
    //
    // Note that this means that we cannot add a byte array of size 2^16 or longer
    // Also note that the network backend also uses 2 bytes as message header,
    // so messages cannot be longer than 2^16 bytes either
    // Let's limit byte arrays to 2^15 bytes here so we don't run into problems
    if (val.size() > (1u << 15))
    {
      LOG_ERROR("Trying to add byte array with size: %d (maximum size is %d)",
                static_cast<int>(val.size()),
                (1u << 15));
      exit(EXIT_FAILURE);
    }

    add<std::uint16_t>(buffer, val.size());
    buffer->insert(buffer->end(), val.begin(), val.end());
  }

  template<typename T>
  bool get(const std::vector<std::uint8_t>& data, int* pos, T* val);

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, std::uint8_t* val)
  {
    if (*pos + sizeof(*val) > data.size()) return false;
    *val = data[*pos];
    *pos += sizeof(*val);
    return true;
  }

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, std::uint16_t* val)
  {
    if (*pos + sizeof(*val) > data.size()) return false;
    auto* tmp = reinterpret_cast<std::uint8_t*>(val);
    std::copy(data.data() + *pos, data.data() + *pos + sizeof(*val), tmp);
    *pos += sizeof(*val);
    return true;
  }

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, std::uint32_t* val)
  {
    if (*pos + sizeof(*val) > data.size()) return false;
    auto* tmp = reinterpret_cast<std::uint8_t*>(val);
    std::copy(data.data() + *pos, data.data() + *pos + sizeof(*val), tmp);
    *pos += sizeof(*val);
    return true;
  }

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, bool* val)
  {
    std::uint8_t val_u8;
    if (!get(data, pos, &val_u8)) return false;
    *val = val_u8 == 1;
    return true;
  }

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, double* val)
  {
    if (*pos + sizeof(*val) > data.size()) return false;
    auto* tmp = reinterpret_cast<std::uint8_t*>(val);
    std::copy(data.data() + *pos, data.data() + *pos + sizeof(*val), tmp);
    *pos += sizeof(*val);
    return true;
  }

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, std::complex<double>* val)
  {
    double real;
    double imag;
    if (!get(data, pos, &real)) return false;
    if (!get(data, pos, &imag)) return false;
    *val = std::complex<double>(real, imag);
    return true;
  }

  template<>
  bool get(const std::vector<std::uint8_t>& data, int* pos, std::vector<std::uint8_t>* val)
  {
    std::uint16_t num_bytes;
    if (!get(data, pos, &num_bytes)) return false;
    if (*pos + num_bytes > static_cast<int>(data.size())) return false;
    val->clear();
    val->insert(val->end(), data.data() + *pos, data.data() + *pos + num_bytes);
    *pos += num_bytes;
    return true;
  }
}

namespace Protocol
{

template<>
std::vector<std::uint8_t> serialize(const Request& request)
{
  std::vector<std::uint8_t> buffer;
  add(&buffer, request.min_c);
  add(&buffer, request.max_c);
  add(&buffer, request.image_width);
  add(&buffer, request.image_height);
  add(&buffer, request.max_iter);
  return buffer;
}

template<>
bool deserialize(const std::vector<std::uint8_t>& data, Request* request)
{
  auto pos = 0;
  if (!get(data, &pos, &request->min_c))        return false;
  if (!get(data, &pos, &request->max_c))        return false;
  if (!get(data, &pos, &request->image_width))  return false;
  if (!get(data, &pos, &request->image_height)) return false;
  if (!get(data, &pos, &request->max_iter))     return false;
  return true;
}

template<>
std::vector<std::uint8_t> serialize(const Response& response)
{
  std::vector<std::uint8_t> buffer;
  add(&buffer, response.pixels);
  add(&buffer, response.last_message);
  return buffer;
}

template<>
bool deserialize(const std::vector<std::uint8_t>& data, Response* response)
{
  auto pos = 0;
  if (!get(data, &pos, &response->pixels))       return false;
  if (!get(data, &pos, &response->last_message)) return false;
  return true;
}

}
