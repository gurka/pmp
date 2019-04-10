#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <cstdint>
#include <vector>

namespace Mandelbrot
{

std::vector<std::uint8_t> compute(float min_c_re,
                                  float min_c_im,
                                  float max_c_re,
                                  float max_c_im,
                                  int x,
                                  int y,
                                  int inf_n);

}

#endif  // MANDELBROT_H_
