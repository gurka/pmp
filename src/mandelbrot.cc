#include "mandelbrot.h"

#include <cmath>

std::vector<std::uint8_t> Mandelbrot::compute(std::complex<double> min_c,
                                              std::complex<double> max_c,
                                              int image_width,
                                              int image_height,
                                              int max_iter)
{
  std::vector<std::uint8_t> pixels;

  // Calculate image pixel size in the complex plane
  // Example: with x=y=10, min_c=(-2.0, -2.0) and max_c=(2.0, 2.0)
  //          we'll get dc=(4.0, 4.0), dx=0.4 and dy=0.4
  //          pixel at 0,0 corresponds to -2.0,-2.0, pixel at 1,1 to -1.6,-1.6
  //          and so on.
  const auto dc = max_c - min_c;
  const auto dx = dc.real() / static_cast<double>(image_width);
  const auto dy = dc.imag() / static_cast<double>(image_height);

  // Iterate over each image pixel
  for (auto py = 0; py < image_height; py++)
  {
    for (auto px = 0; px < image_width; px++)
    {
      // Mandelbrot set algorithm
      const auto c = min_c + std::complex<double>(px * dx, py * dy);
      auto z = std::complex<double>(0.0f, 0.0f);
      int n = 0;
      while (n < max_iter && std::abs(z) < 2.0f)
      {
        z = std::pow(z, 2) + c;
        n += 1;
      }

      if (n == max_iter)
      {
        pixels.push_back(0);  // black
      }
      else
      {
        // Implicit modulus 256 since pixels are std::uint8_t
        pixels.push_back(n);
      }
    }
  }

  return pixels;
}
