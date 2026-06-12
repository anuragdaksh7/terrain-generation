#include "terrain/Simulation/Tectonic.hpp"
#include <random>
#include <iostream>

namespace terrain {
  void initializePlates(MapGraph& graph, double waterRatio) {
    std::cout << "\n[Tectonic] Initializing Plates (Target Water Ratio: " << waterRatio * 100 << "%)..." << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::uniform_real_distribution<double> elevDist(-1.0, 1.0);

    int oceanCount = 0;
    int landCount = 0;

    for (auto& center: graph.centers) {

      if (dist(gen) < waterRatio) {
        center.isWater = true;
        center.isOcean = true;
        center.elevation = -1.0 * elevDist(gen);
        oceanCount++;
      } else {
        center.isWater = false;
        center.isOcean = false;
        center.elevation = elevDist(gen);
        landCount++;
      }
    }

    std::cout << "[Tectonic] World generated with " << oceanCount << " Ocean plates and " << landCount << " Continental plates." << std::endl;
  }
}
