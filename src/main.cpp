#include <iostream>
#include <vector>
#include "terrain/Core/Vector2.hpp"
#include "terrain/Map/Generator.hpp"
#include "terrain/Simulation/Tectonic.hpp"
#include "terrain/Simulation/Erosion.hpp"

const double HEIGHT = 1000.0;
const double WIDTH = 1000.0;
const int NUM_POINTS = 499;

int main() {
  std::cout << "--- Terrain Generator Started ---" << std::endl;

  // 1. Generate points
  std::vector<terrain::Vector2> seeds = terrain::generateSeedPoints(NUM_POINTS, WIDTH, HEIGHT);

  // 2. Build the Voronoi Map Graph
  terrain::MapGraph graph = terrain::buildVoronoiMap(seeds, WIDTH, HEIGHT);

  // 3. Run the Tectonic Initialization (Make 70% of the world oceans)
  terrain::initializePlates(graph, 0.7);

  // 3.5. Optional: Smooth the terrain to create more natural landmasses and coastlines
  terrain::smoothElevations(graph, 4);

  // 4. Interpolate and blend plate properties to find corner elevations
  terrain::assignCornerElevations(graph);

  // 5. Fluvial Erosion (Rivers!)
  terrain::calculateDownslopes(graph);
  terrain::simulateRivers(graph);

  // 6. Render a gorgeous solid snapshot map to verify your world shapes!
  terrain::renderTectonicMap(graph, WIDTH, HEIGHT);

  return 0;
}
