#include "terrain/Map/Generator.hpp"
#include "terrain/IO/ImageExport.hpp" 
#include <random>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm> // Required for std::max / std::min

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

namespace terrain {

    // --- The 100% Uncrashable Bresenham's Line Algorithm ---
    void drawLine(std::vector<unsigned char>& image, int width, int height, int x0, int y0, int x1, int y1) {
        // Force coordinates to stay inside the image bounds
        x0 = std::max(0, std::min(width - 1, x0));
        y0 = std::max(0, std::min(height - 1, y0));
        x1 = std::max(0, std::min(width - 1, x1));
        y1 = std::max(0, std::min(height - 1, y1));

        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            int index = (y0 * width + x0) * 3;
            
            // Absolute foolproof safety check: Only draw if the index is physically inside the vector
            if (index >= 0 && index + 2 < static_cast<int>(image.size())) {
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
        std::uniform_real_distribution<double> distX(0.0, width);
        std::uniform_real_distribution<double> distY(0.0, height);
        for (int i = 0; i < count; ++i) {
            points.push_back({distX(gen), distY(gen)});
        }
        return points;
    }

    void buildVoronoiMap(const std::vector<Vector2>& points, double width, double height) {
        std::cout << "[Step 1] Initializing Voronoi..." << std::endl;

        std::vector<jcv_point> jcv_points(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            jcv_points[i].x = static_cast<float>(points[i].x);
            jcv_points[i].y = static_cast<float>(points[i].y);
        }

        jcv_rect bounding_box = { {0.0f, 0.0f}, {static_cast<float>(width), static_cast<float>(height)} };
        jcv_diagram diagram;
        memset(&diagram, 0, sizeof(jcv_diagram));
        
        jcv_diagram_generate(jcv_points.size(), jcv_points.data(), &bounding_box, 0, &diagram);

        std::cout << "[Step 2] Allocating image memory..." << std::endl;
        int w = static_cast<int>(width);
        int h = static_cast<int>(height);
        std::vector<unsigned char> pixels(w * h * 3, 0); 

        std::cout << "[Step 3] Drawing lines..." << std::endl;
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

        std::cout << "[Step 4] Saving PPM to disk..." << std::endl;
        terrain::savePPM("voronoi_map.ppm", pixels, w, h);

        std::cout << "[Step 5] Freeing library memory..." << std::endl;
        jcv_diagram_free(&diagram);

        std::cout << "[DONE] Successfully generated!" << std::endl;
    }

} // namespace terrain