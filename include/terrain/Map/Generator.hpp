#pragma once

#include <vector>
#include "terrain/Core/Vector2.hpp"

namespace terrain {
  std::vector<Vector2> generateSeedPoints(int count, double width, double height);

  void buildVoronoiMap(const std::vector<Vector2>& points, double width, double height);
}