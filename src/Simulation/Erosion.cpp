#include "terrain/Simulation/Erosion.hpp"
#include <iostream>
#include <algorithm>

namespace terrain
{

  void calculateDownslopes(MapGraph &graph)
  {
    std::cout << "[Erosion] Calculating Downslopes..." << std::endl;

    for (auto &corner : graph.corners)
    {
      int lowestNeighbor = -1;
      double lowestElevation = corner.elevation;

      for (int adjacentIndex : corner.adjacent)
      {
        if (graph.corners[adjacentIndex].elevation < lowestElevation)
        {
          lowestElevation = graph.corners[adjacentIndex].elevation;
          lowestNeighbor = adjacentIndex;
        }
      }
      corner.downslope = lowestNeighbor;
    }
  }

  void simulateRivers(MapGraph &graph)
  {
    std::cout << "[Erosion] Simulating Water Flow..." << std::endl;

    for (auto &corner : graph.corners)
    {
      corner.river = 0;
    }

    std::vector<Corner *> sortedCorners;
    for (auto &corner : graph.corners)
    {
      sortedCorners.push_back(&corner);
    }

    std::sort(sortedCorners.begin(), sortedCorners.end(), [](Corner *a, Corner *b)
              { return a->elevation > b->elevation; });

    // 1. Drop rain and flow downhill
    for (Corner *c : sortedCorners)
    {
      if (c->elevation > 0.001)
      {
        c->river += 1;
      }
      if (c->elevation <= 0.001)
      {
        continue;
      }
      if (c->downslope != -1)
      {
        graph.corners[c->downslope].river += c->river;
      }
    }

    // --- 2. THE NUCLEAR SCRUB ---
    // Absolutely guarantee that no rivers exist in the ocean
    for (auto &corner : graph.corners)
    {
      bool touchesOcean = false;

      // Check if this corner touches ANY blue ocean plates
      for (int centerIndex : corner.touches)
      {
        // Safety check to prevent any out-of-bounds access
        if (centerIndex >= 0 && centerIndex < graph.centers.size())
        {
          if (graph.centers[centerIndex].isOcean)
          {
            touchesOcean = true;
            break;
          }
        }
      }

      // Forcibly delete the river data so the renderer physically cannot draw it
      if (touchesOcean || corner.elevation <= 0.0)
      {
        corner.river = 0;
      }
    }

    std::cout << "[Erosion] Rivers carved and coastlines strictly scrubbed!" << std::endl;
  }

  void calculateMoisture(MapGraph& graph) {
    std::cout << "[Biome] Calculating Global Moisture..." << std::endl;

    // 1. Seed the initial moisture sources
    for (auto& center : graph.centers) {
      if (center.isOcean) {
        center.moisture = 1.0; // Oceans are 100% wet
      } else {
        center.moisture = 0.0; // Assume dry land first
          
        // Check if any of this plate's corners contain a river
        for (int cornerIdx : center.corners) {
          if (cornerIdx >= 0 && cornerIdx < graph.corners.size()) {
            if (graph.corners[cornerIdx].river > 0) {
              center.moisture = 1.0; // Rivers make the land 100% wet
              break;
            }
          }
        }
      }
    }

    // 2. Diffuse the moisture (Wind blowing rain across the continent)
    // Doing this 3 times spreads the water nicely into the deep mainland
    for (int pass = 0; pass < 3; ++pass) {
      std::vector<double> newMoisture(graph.centers.size());

      for (auto& center : graph.centers) {
        double sum = center.moisture;
        int count = 1;

        for (int neighborIdx : center.neighbors) {
          if (neighborIdx >= 0 && neighborIdx < graph.centers.size()) {
            sum += graph.centers[neighborIdx].moisture;
            count++;
          }
        }
        newMoisture[center.index] = sum / count;
      }

      // Apply the spread moisture
      for (auto& center : graph.centers) {
        center.moisture = newMoisture[center.index];
      }
    }
    std::cout << "[Biome] Moisture diffused successfully!" << std::endl;
  }
} // namespace terrain