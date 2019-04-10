#include "mandelbrot.h"

#include <cmath>
#include <complex>

std::vector<std::uint8_t> Mandelbrot::compute(float min_c_re,
                                              float min_c_im,
                                              float max_c_re,
                                              float max_c_im,
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
  const auto dx = (max_c_re - min_c_re) / static_cast<float>(x);
  const auto dy = (max_c_im - min_c_im) / static_cast<float>(y);

  // Iterate over each pixel
  for (auto py = 0; py < y; py++)
  {
    for (auto px = 0; px < x; px++)
    {
      const auto c = std::complex<float>(min_c_re + (px * dx),
                                         min_c_im + (py * dy));
      auto z = std::complex<float>(0.0f, 0.0f);
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
        // TODO: Map 0..inf_n to 0..255 instead?
        // Implictly cast int to std::uint8_t (i.e. n modulus 255)
        pixels.push_back(n);
      }
    }
  }

  return pixels;
}
