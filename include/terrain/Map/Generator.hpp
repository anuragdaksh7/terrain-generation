#pragma once

#include <vector>
#include "terrain/Core/Vector2.hpp"
#include "terrain/Map/Graph.hpp"

namespace terrain {
  std::vector<Vector2> generateSeedPoints(int count, double width, double height);

  MapGraph buildVoronoiMap(const std::vector<Vector2>& points, double width, double height);
  void renderTectonicMap(const MapGraph& graph, double width, double height);
}