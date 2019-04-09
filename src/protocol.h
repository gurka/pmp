#ifndef PROTOCOL_H_
#define PROTOCOL_H_

namespace Protocol
{

// This simple protocl requires both server and client to be built on the same
// platform and architecture, otherwise the struct layout and size might differ
// and everything will explode

struct Request
{
  float min_c_re;
  float min_c_im;
  float max_c_re;
  float max_c_im;
  int x;
  int y;
  int inf_n;
};

struct Response
{
  // TODO
  char text[32];
};

}

#endif  // PROTOCOL_H_
