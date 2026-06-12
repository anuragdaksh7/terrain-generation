#pragma once

#include <vector>
#include "terrain/Core/Vector2.hpp"

namespace terrain {
  struct Center;
  struct Corner;
  struct Edge;

  struct Center {
    int index;
    Vector2 position;

    std::vector<int> neighbors;
    std::vector<int> borders;
    std::vector<int> corners;

    double elevation = 0.0;
    double moisture = 0.0;
    bool isWater = false;
    bool isOcean = false;
  };

  struct Corner {
    int index;
    Vector2 position;
    
    std::vector<int> touches;
    std::vector<int> protrudes;
    std::vector<int> adjacent;

    double elevation = 0.0;
    double moisture = 0.0;
    int river = 0;
    int downslope = -1;
  };

  struct Edge {
    int index;
    
    int v0 = -1;
    int v1 = -1;

    int d0 = -1;
    int d1 = -1;

    int river = 0;
  };

  struct MapGraph {
    std::vector<Center> centers;
    std::vector<Corner> corners;
    std::vector<Edge> edges;
  };
}
