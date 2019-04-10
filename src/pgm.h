#ifndef PGM_H_
#define PGM_H_

#include <cstdint>
#include <string>

namespace PGM
{

void write_pgm(const std::string& filename, int width, int height, const std::uint8_t* pixels);

}

#endif  // PGM_H_
