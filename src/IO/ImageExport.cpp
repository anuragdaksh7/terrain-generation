#include "terrain/IO/ImageExport.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace terrain {
  void savePNG(const std::string& filename, const std::vector<unsigned char>& pixels, int width, int height) {
    stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
  }
}
