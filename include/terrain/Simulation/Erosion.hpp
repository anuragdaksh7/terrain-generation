#pragma once

#include "terrain/Map/Graph.hpp"

namespace terrain {
  void calculateDownslopes(MapGraph& graph);
  void simulateRivers(MapGraph& graph);
  void calculateMoisture(MapGraph& graph);
}
