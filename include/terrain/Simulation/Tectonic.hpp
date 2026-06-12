#pragma once

#include "terrain/Map/Graph.hpp"

namespace terrain {
  void initializePlates(MapGraph& graph, double waterRatio = 0.6);
}
