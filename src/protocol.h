#ifndef PROTOCOL_H_
#define PROTOCOL_H_

namespace Protocol
{

// This simple protocol requires both server and client to be built on the same
// platform and architecture, otherwise the struct layout and size might differ
// and everything will explode

// TODO: Seralize/deseralize to something like json (https://github.com/nlohmann/json)

struct Request
{
  double min_c_re;
  double min_c_im;
  double max_c_re;
  double max_c_im;
  int image_width;
  int image_height;
  int max_iter;
};

struct Response
{
  int num_pixels;
  std::uint8_t pixels[(1 << 16) - 64];  // -64 is arbitrary and used to make sure that
                                        // the struct is smaller than 2^16 - 1
  int last_message;  // 0 = false, !0 = true
};

// Maximum data length is 2^16 - 1, so make sure that the struct is not too large
static_assert(sizeof(Response) < (1 << 16) - 1, "struct Response is too large");

}

#endif  // PROTOCOL_H_
