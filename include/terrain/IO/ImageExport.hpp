#pragma once

#include <vector>
#include <string>

namespace terrain {
  void savePNG(const std::string& filename, const std::vector<unsigned char>& pixels, int width, int height);
}
