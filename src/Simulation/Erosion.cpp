#include "terrain/Simulation/Erosion.hpp"
#include <iostream>
#include <algorithm>

namespace terrain {
  void calculateDownslopes(MapGraph& graph) {
    std::cout << "[Erosion] Calculating downslopes..." << std::endl;

    for (auto& corner : graph.corners) {
      int lowestNeighbor = -1;
      double lowestElevation = corner.elevation;

      for (int adjacentIndex : corner.adjacent) {
        if (graph.corners[adjacentIndex].elevation < lowestElevation) {
          lowestElevation = graph.corners[adjacentIndex].elevation;
          lowestNeighbor = adjacentIndex;
        }
      }

      corner.downslope = lowestNeighbor;
    }
  }

  void simulateRivers(MapGraph& graph) {
    std::cout << "[Erosion] Simulating Water Flow..." << std::endl;

    for (auto& corner : graph.corners) {
      corner.river = 0;
    }

    std::vector<Corner*> sortedCorners;
    for (auto& corner : graph.corners) {
      sortedCorners.push_back(&corner);
    }

    std::sort(sortedCorners.begin(), sortedCorners.end(), [](Corner* a, Corner* b) {
      return a->elevation > b->elevation;
    });

    for (Corner* c : sortedCorners) {
      if (c->elevation > 0.001) {
        c->river += 1;
      }
      
      if (c->elevation <= 0.001) {
        continue; // Skip passing water to the downslope
      }

      if (c->downslope != -1) {
        graph.corners[c->downslope].river += c->river;
      }
    }

    std::cout << "[Erosion] Rivers carved!" << std::endl;
  }
}
