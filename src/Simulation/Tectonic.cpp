#include "terrain/Simulation/Tectonic.hpp"
#include <random>
#include <iostream>

namespace terrain
{
  void initializePlates(MapGraph &graph, double waterRatio)
  {
    std::cout << "\n[Tectonic] Initializing Plates (Target Water Ratio: " << waterRatio * 100 << "%)..." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::uniform_real_distribution<double> elevDist(0.1, 1.0);

    int oceanCount = 0;
    int landCount = 0;

    for (auto &center : graph.centers)
    {

      if (dist(gen) < waterRatio)
      {
        center.isWater = true;
        center.isOcean = true;
        center.elevation = -1.0 * elevDist(gen);
        oceanCount++;
      }
      else
      {
        center.isWater = false;
        center.isOcean = false;
        center.elevation = elevDist(gen);
        landCount++;
      }
    }

    std::cout << "[Tectonic] World generated with " << oceanCount << " Ocean plates and " << landCount << " Continental plates." << std::endl;
  }

  void assignCornerElevations(MapGraph &graph)
  {
    std::cout << "[Tectonic] Calculating Corner Elevations..." << std::endl;

    for (auto &corner : graph.corners)
    {

      if (corner.touches.empty())
        continue;

      double sumElevation = 0.0;
      bool touchesOcean = false;
      bool touchesLand = false;

      // Look at every plate (Center) this corner touches
      for (int centerIndex : corner.touches)
      {
        const Center &adjacentPlate = graph.centers[centerIndex];

        sumElevation += adjacentPlate.elevation;

        if (adjacentPlate.isOcean)
          touchesOcean = true;
        else
          touchesLand = true;
      }

      // The exact average of the touching plates
      corner.elevation = sumElevation / corner.touches.size();

      // Optional physics bonus:
      // If a corner touches BOTH land and ocean, it is officially a Coastline.
      // You can use this later to spawn beaches or coastal cities!
      if (touchesOcean && touchesLand)
      {
        // Force coastlines to be exactly at sea level (0.0) for a clean look
        corner.elevation = 0.0;
      }
    }

    std::cout << "[Tectonic] Assigned elevations to " << graph.corners.size() << " corners." << std::endl;
  }

  void smoothElevations(MapGraph &graph, int iterations)
  {
    std::cout << "[Tectonic] Smoothing Terrain (" << iterations << " passes)..." << std::endl;

    for (int i = 0; i < iterations; ++i)
    {
      // Create a temporary array to hold the new smoothed heights
      std::vector<double> newElevations(graph.centers.size());

      for (auto &center : graph.centers)
      {
        double sum = center.elevation;
        int count = 1;

        for (int neighborIdx : center.neighbors)
        {
          // Only smooth land with land, and ocean with ocean.
          // This prevents coastlines from blurring into muddy swamps!
          if (graph.centers[neighborIdx].isOcean == center.isOcean)
          {
            sum += graph.centers[neighborIdx].elevation;
            count++;
          }
        }
        newElevations[center.index] = sum / count;
      }

      // Apply the smoothed heights back to the map
      for (auto &center : graph.centers)
      {
        center.elevation = newElevations[center.index];
      }
    }
  }

  void buildMountainRanges(MapGraph &graph, int numRanges) {
    std::cout << "[Tectonic] Uplifting Mountain Ranges..." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<int> landPlates;
    for (const auto& center : graph.centers) {
      if (!center.isOcean) {
        landPlates.push_back(center.index);
      }
    }

    if (landPlates.empty()) return;

    std::uniform_int_distribution<int> landDist(0, landPlates.size() - 1);
    std::uniform_int_distribution<int> lengthDist(10, 30);

    for (int i = 0; i< numRanges; ++i) {
      int currentPlate = landPlates[landDist(gen)];
      int rangeLength = lengthDist(gen);

      for (int step = 0; step < rangeLength; ++step) {
        graph.centers[currentPlate].elevation += 4.0;

        std::vector<int> landNeighbors;
        for (int neighborIdx : graph.centers[currentPlate].neighbors) {
          if (neighborIdx >= 0 && neighborIdx < graph.centers.size()) {
            if (!graph.centers[neighborIdx].isOcean) {
              landNeighbors.push_back(neighborIdx);
            }
          }
        }

        if (landNeighbors.empty()) break;

        std::uniform_int_distribution<int> neighborDist(0, landNeighbors.size() - 1);
        currentPlate = landNeighbors[neighborDist(gen)];
      }
    }

    std::cout << "[Tectonic] Built " << numRanges << " mountain ridges!" << std::endl;
  }

  void normalizeElevations(MapGraph& graph) {
    std::cout << "[Tectonic] Normalizing land elevations to 1.0 max..." << std::endl;
    
    double maxElevation = 0.0;
    
    // Pass 1: Find the absolute highest peak on the map
    for (const auto& center : graph.centers) {
      if (!center.isOcean && center.elevation > maxElevation) {
        maxElevation = center.elevation;
      }
    }

    // Pass 2: Divide all land by that peak, scaling everything perfectly to 0.0 -> 1.0
    if (maxElevation > 0.0) {
      for (auto& center : graph.centers) {
        if (!center.isOcean) {
          center.elevation = center.elevation / maxElevation;
        }
      }
    }
  }
}
