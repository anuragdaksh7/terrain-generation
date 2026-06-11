#include "terrain/Map/Generator.hpp"
#include "terrain/IO/ImageExport.hpp"
#include <random>
#include <iostream>
#include <cstring>

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

namespace terrain {
  // --- Fixed Helper function: Safe Bresenham's Line Algorithm ---
void drawLine(std::vector<unsigned char>& image, int width, int height, int x0, int y0, int x1, int y1) {
    // 1. Strict Boundary Clamping: Ensure all coordinates are strictly within [0, width-1] and [0, height-1]
    if (x0 < 0) x0 = 0; if (x0 >= width) x0 = width - 1;
    if (y0 < 0) y0 = 0; if (y0 >= height) y0 = height - 1;
    if (x1 < 0) x1 = 0; if (x1 >= width) x1 = width - 1;
    if (y1 < 0) y1 = 0; if (y1 >= height) y1 = height - 1;

    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        // Double-check bounds to prevent buffer overflows
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            int index = (y0 * width + x0) * 3;
            image[index]     = 255; // R
            image[index + 1] = 255; // G
            image[index + 2] = 255; // B
        }
        
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

  std::vector<Vector2> generateSeedPoints(int count, double width, double height) {
    std::vector<Vector2> points;
    points.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<double> disX(0.0, width);
    std::uniform_real_distribution<double> disY(0.0, height);

    for (int i = 0; i< count; ++i) {
      points.push_back({disX(gen), disY(gen)});
    }
    return points;
  }

  void buildVoronoiMap(const std::vector<Vector2>& points, double width, double height) {
    std::cout<< "Building Voronoi diagram..." << std::endl;

    std::vector<jcv_point> jcv_points(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
      jcv_points[i].x = static_cast<float>(points[i].x);
      jcv_points[i].y = static_cast<float>(points[i].y);
    }

    jcv_rect bounding_box = { {0.0f, 0.0f}, {static_cast<float>(width), static_cast<float>(height)} };

    jcv_diagram diagram;
    memset(&diagram, 0, sizeof(jcv_diagram));

    jcv_diagram_generate(jcv_points.size(), jcv_points.data(), &bounding_box, 0, &diagram);

    std::cout << "Success! Generated Vorornoi map with " << diagram.numsites << " sites" << std::endl;
    std::cout << "Rendering image..." << std::endl;

    int w = static_cast<int>(width);
    int h = static_cast<int>(height);
    std::vector<unsigned char> pixels(w * h * 3, 0);

    const jcv_site* sites = jcv_diagram_get_sites(&diagram);
    for (int i = 0; i < diagram.numsites; ++i) {
        const jcv_site* site = &sites[i];
        const jcv_graphedge* e = site->edges;
        
        while (e) {
            drawLine(pixels, w, h, 
                      static_cast<int>(e->pos[0].x), static_cast<int>(e->pos[0].y),
                      static_cast<int>(e->pos[1].x), static_cast<int>(e->pos[1].y));
            e = e->next;
        }
    }

    // Save the image using our new IO module!
    terrain::savePNG("voronoi_map.png", pixels, w, h);
    std::cout << "Saved to 'voronoi_map.png'!" << std::endl;

    jcv_diagram_free(&diagram);
  }
}
