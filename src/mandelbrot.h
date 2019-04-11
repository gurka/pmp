#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <cstdint>
#include <complex>
#include <vector>

namespace Mandelbrot
{

/**
 * @brief Computes (renders) the Mandelbrot set
 *
 * @param[in]  min_c         Minimum complex value
 * @param[in]  max_c         Maximum complex value
 * @param[in]  image_width   Width of output image
 * @param[in]  image_height  Height of output image
 * @param[in]  max_iter      Maximum number of iterations per sample/point
 *
 * @return Image pixels (8bpp)
 *         The number of pixels is image_width * image_height
 */
std::vector<std::uint8_t> compute(std::complex<double> min_c,
                                  std::complex<double> max_c,
                                  int image_width,
                                  int image_height,
                                  int max_iter);

}

#endif  // MANDELBROT_H_
