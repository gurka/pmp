#ifndef PGM_H_
#define PGM_H_

#include <cstdint>
#include <string>

namespace PGM
{

/**
 * @brief Writes a PGM file with the given arguments
 *
 * @param[in]  filename  Name of the file to write
 * @param[in]  width     Width in pixels of the image
 * @param[in]  height    Height in pixels of the image
 * @param[in]  pixels    Pointer to array of pixels (8bpp)
 *                       The number of pixels in the array must be width * height
 */
void write_pgm(const std::string& filename, int width, int height, const std::uint8_t* pixels);

}

#endif  // PGM_H_
