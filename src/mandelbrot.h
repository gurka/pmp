#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <cstdint>
#include <vector>

namespace Mandelbrot
{

/**
 * @brief Computes (renders) the Mandelbrot set
 *
 * @param[in]  min_c_re  Minimum complex real value
 * @param[in]  min_c_im  Minimum complex imaginary value
 * @param[in]  max_c_re  Maximum complex real value
 * @param[in]  max_c_im  Maximum complex imaginary value
 * @param[in]  x         Width of output image
 * @param[in]  y         Height of output image
 * @param[in]  inf_n     Maximum number of iterations per sample
 *
 * @return Image pixels (8bpp). The number of pixels is x * y.
 */
std::vector<std::uint8_t> compute(double min_c_re,  // TODO: use std::complex<double> for min and max?
                                  double min_c_im,
                                  double max_c_re,
                                  double max_c_im,
                                  int x,
                                  int y,
                                  int inf_n);

}

#endif  // MANDELBROT_H_
