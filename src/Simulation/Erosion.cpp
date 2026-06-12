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
} // namespace terrain