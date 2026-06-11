#pragma once

#include <vector>
#include <string>

namespace terrain {
  void savePPM(const std::string& filename, const std::vector<unsigned char>& pixels, int width, int height);
}
