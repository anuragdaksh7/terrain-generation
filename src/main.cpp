#include <iostream>
#include <vector>
#include "terrain/Core/Vector2.hpp"
#include "terrain/Map/Generator.hpp"
#include "terrain/Simulation/Tectonic.hpp"

const double HEIGHT = 1000.0;
const double WIDTH = 1000.0;
const int NUM_POINTS = 500;

int main() {
  std::cout << "--- Terrain Generator Started ---" << std::endl;

  // 1. Generate points
  std::vector<terrain::Vector2> seeds = terrain::generateSeedPoints(NUM_POINTS, WIDTH, HEIGHT);

  // 2. Build the Voronoi Map Graph
  terrain::MapGraph graph = terrain::buildVoronoiMap(seeds, WIDTH, HEIGHT);

  // 3. Run the Tectonic Initialization (Make 70% of the world oceans)
  terrain::initializePlates(graph, 0.7);

  return 0;
}
