#include "pgm.h"

#include <fstream>

void PGM::write_pgm(const std::string& filename, int width, int height, const std::uint8_t* pixels)
{
  auto fstream = std::ofstream(filename, std::ios::binary);

  // Write header
  fstream << "P5\n"
          << width << " " << height << "\n"
          << 255 << "\n";

  // Write pixels
  fstream.write(reinterpret_cast<const char*>(pixels), width * height);
}
