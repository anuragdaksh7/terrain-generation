#pragma once

#include "terrain/Map/Graph.hpp"

namespace terrain {
  void initializePlates(MapGraph& graph, double waterRatio = 0.6);
  void smoothElevations(MapGraph& graph, int iterations = 3);
  void assignCornerElevations(MapGraph& graph);
}
