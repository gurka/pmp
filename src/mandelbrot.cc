#include "mandelbrot.h"

#include <cmath>
#include <complex>

std::vector<std::uint8_t> Mandelbrot::compute(double min_c_re,
                                              double min_c_im,
                                              double max_c_re,
                                              double max_c_im,
                                              int x,
                                              int y,
                                              int inf_n)
{
  std::vector<std::uint8_t> pixels;

  // TODO: Verify that the ratio x/y is the same as re/im?

  // Width and height of a pixel in the complex plane
  // Example: with x=y=10, min_c_re=min_c=im=-2.0 and max_c_re=max_c_im=2.0
  //          we'll get dx=0.4 and dy=0.4
  //          pixel at 0,0 corresponds to -2.0,-2.0, pixel at 1,1 to -1.6,-1.6
  //          and so on.
  const auto dx = (max_c_re - min_c_re) / static_cast<double>(x);
  const auto dy = (max_c_im - min_c_im) / static_cast<double>(y);

  // Iterate over each pixel
  for (auto py = 0; py < y; py++)
  {
    for (auto px = 0; px < x; px++)
    {
      const auto c = std::complex<double>(min_c_re + (px * dx),
                                          min_c_im + (py * dy));
      auto z = std::complex<double>(0.0f, 0.0f);
      int n = 0;
      while (n < inf_n && std::abs(z) < 2.0f)
      {
        z = std::pow(z, 2) + c;
        n += 1;
      }

      if (n == inf_n)
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
