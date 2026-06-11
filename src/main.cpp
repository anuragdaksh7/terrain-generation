#include <iostream>
#include <vector>
#include "terrain/Core/Vector2.hpp"
#include "terrain/Map/Generator.hpp"

const double HEIGHT = 1000;
const double WIDTH = 1000;
const int NUM_POINTS = 7;

int main() {
  std::cout << "Generating "<< NUM_POINTS << " voronoi seed points..." << std::endl;

  std::vector<terrain::Vector2> seeds = terrain::generateSeedPoints(NUM_POINTS, WIDTH, HEIGHT);

  terrain::buildVoronoiMap(seeds, WIDTH, HEIGHT);

  return 0;
}
