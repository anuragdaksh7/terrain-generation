#include "terrain/Map/Generator.hpp"
#include "terrain/IO/ImageExport.hpp"
#include "terrain/Map/Graph.hpp"
#include <random>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm> // Required for std::find, std::min, std::max
#include <map>

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

namespace terrain
{

    // --- The 100% Uncrashable Bresenham's Line Algorithm ---
    // --- Updated Helper function: Color-Aware Bresenham's Line Algorithm ---
    void drawLine(std::vector<unsigned char> &image, int width, int height,
                  int x0, int y0, int x1, int y1,
                  unsigned char r = 255, unsigned char g = 255, unsigned char b = 255)
    { // <-- Added parameters

        x0 = std::max(0, std::min(width - 1, x0));
        y0 = std::max(0, std::min(height - 1, y0));
        x1 = std::max(0, std::min(width - 1, x1));
        y1 = std::max(0, std::min(height - 1, y1));

        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true)
        {
            int index = (y0 * width + x0) * 3;

            if (index >= 0 && index + 2 < static_cast<int>(image.size()))
            {
                image[index] = r;     // <-- Use custom R
                image[index + 1] = g; // <-- Use custom G
                image[index + 2] = b; // <-- Use custom B
            }

            if (x0 == x1 && y0 == y1)
                break;
            e2 = 2 * err;
            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    // --- Seed Point Generator ---
    std::vector<Vector2> generateSeedPoints(int count, double width, double height)
    {
        std::vector<Vector2> points;
        points.reserve(count);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> distX(0.0, width);
        std::uniform_real_distribution<double> distY(0.0, height);
        for (int i = 0; i < count; ++i)
        {
            points.push_back({distX(gen), distY(gen)});
        }
        return points;
    }

    // --- Master Voronoi Map Graph Generator ---
    MapGraph buildVoronoiMap(const std::vector<Vector2> &points, double width, double height)
    {
        std::cout << "[Step 1] Initializing Voronoi..." << std::endl;

        std::vector<jcv_point> jcv_points(points.size());
        for (size_t i = 0; i < points.size(); ++i)
        {
            jcv_points[i].x = static_cast<float>(points[i].x);
            jcv_points[i].y = static_cast<float>(points[i].y);
        }

        jcv_rect bounding_box = {{0.0f, 0.0f}, {static_cast<float>(width), static_cast<float>(height)}};
        jcv_diagram diagram;
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(jcv_points.size(), jcv_points.data(), &bounding_box, 0, &diagram);

        std::cout << "[Step 2] Allocating image memory..." << std::endl;
        int w = static_cast<int>(width);
        int h = static_cast<int>(height);
        std::vector<unsigned char> pixels(w * h * 3, 0);

        std::cout << "[Step 2.5] Converting to Map Graph and Linking Neighbors..." << std::endl;
        terrain::MapGraph graph;
        graph.centers.resize(points.size());
        std::map<std::pair<int, int>, int> cornerLookup;

        const jcv_site *sites = jcv_diagram_get_sites(&diagram);
        for (int i = 0; i < diagram.numsites; ++i)
        {
            const jcv_site *site = &sites[i];

            // 1. Create the Center (The Tectonic Plate)
            Center c;
            c.index = site->index;
            c.position = {site->p.x, site->p.y};

            // 2. Extract its borders, corners, and neighbors
            const jcv_graphedge *e = site->edges;
            while (e)
            {
                // --- Process Corner 0 ---
                int x0 = static_cast<int>(std::round(e->pos[0].x * 100.0));
                int y0 = static_cast<int>(std::round(e->pos[0].y * 100.0));
                std::pair<int, int> p0 = {x0, y0};

                int v0_index;
                if (cornerLookup.find(p0) == cornerLookup.end())
                {
                    Corner corner;
                    corner.index = graph.corners.size();
                    corner.position = {e->pos[0].x, e->pos[0].y};
                    graph.corners.push_back(corner);
                    cornerLookup[p0] = corner.index;
                    v0_index = corner.index;
                }
                else
                {
                    v0_index = cornerLookup[p0];
                }

                // --- Process Corner 1 ---
                int x1 = static_cast<int>(std::round(e->pos[1].x * 100.0));
                int y1 = static_cast<int>(std::round(e->pos[1].y * 100.0));
                std::pair<int, int> p1 = {x1, y1};

                int v1_index;
                if (cornerLookup.find(p1) == cornerLookup.end())
                {
                    Corner corner;
                    corner.index = graph.corners.size();
                    corner.position = {e->pos[1].x, e->pos[1].y};
                    graph.corners.push_back(corner);
                    cornerLookup[p1] = corner.index;
                    v1_index = corner.index;
                }
                else
                {
                    v1_index = cornerLookup[p1];
                }
                if (std::find(graph.corners[v0_index].adjacent.begin(), graph.corners[v0_index].adjacent.end(), v1_index) == graph.corners[v0_index].adjacent.end())
                {
                    graph.corners[v0_index].adjacent.push_back(v1_index);
                }
                if (std::find(graph.corners[v1_index].adjacent.begin(), graph.corners[v1_index].adjacent.end(), v0_index) == graph.corners[v1_index].adjacent.end())
                {
                    graph.corners[v1_index].adjacent.push_back(v0_index);
                }

                // --- Link Corners to the Center ---
                c.corners.push_back(v0_index);
                c.corners.push_back(v1_index);

                // --- Link the Center back to the Corners (Two-Way Relationships) ---
                if (std::find(graph.corners[v0_index].touches.begin(), graph.corners[v0_index].touches.end(), c.index) == graph.corners[v0_index].touches.end())
                {
                    graph.corners[v0_index].touches.push_back(c.index);
                }
                if (std::find(graph.corners[v1_index].touches.begin(), graph.corners[v1_index].touches.end(), c.index) == graph.corners[v1_index].touches.end())
                {
                    graph.corners[v1_index].touches.push_back(c.index);
                }

                // --- Link Neighboring Plates ---
                if (e->neighbor)
                {
                    c.neighbors.push_back(e->neighbor->index);
                }

                e = e->next;
            }

            // Save our fully linked plate to the graph
            graph.centers[c.index] = c;
        }

        std::cout << "Graph Linked! Total Unique Corners Found: " << graph.corners.size() << std::endl;

        std::cout << "[Step 5] Freeing library memory..." << std::endl;
        jcv_diagram_free(&diagram);

        std::cout << "[DONE] Graph structure generated successfully!" << std::endl;
        return graph;
    }

    // --- Separate Function: Shading and Exporting Tectonic Maps ---
    void renderTectonicMap(const MapGraph &graph, double width, double height)
    {
        std::cout << "[Step 3] Shading Map Pixels based on Elevations..." << std::endl;
        int w = static_cast<int>(width);
        int h = static_cast<int>(height);
        std::vector<unsigned char> pixels(w * h * 3, 0);

        // Loop through every single pixel on the screen
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {

                // Find the closest Voronoi center to this pixel
                int closestCenterIndex = 0;
                double minDistance = 1e9;

                for (const auto &center : graph.centers)
                {
                    if (center.corners.empty()) continue;
                    
                    double dx = center.position.x - x;
                    double dy = center.position.y - y;
                    double distSq = dx * dx + dy * dy;

                    if (distSq < minDistance)
                    {
                        minDistance = distSq;
                        closestCenterIndex = center.index;
                    }
                }

                // Get the elevation data of the plate owning this pixel
                const auto &cell = graph.centers[closestCenterIndex];
                int pixelIdx = (y * w + x) * 3;

                if (cell.isOcean)
                {
                    // OCEAN: Dark blue to light cyan based on depth
                    int depth = static_cast<int>(std::abs(cell.elevation) * 150);
                    pixels[pixelIdx] = 0;                                    // R
                    pixels[pixelIdx + 1] = std::max(0, 150 - depth);         // G
                    pixels[pixelIdx + 2] = std::max(100, 255 - (depth / 2)); // B
                }
                else
                {
                    // LANDMASS: Green -> Brown -> White based on height
                    int height = static_cast<int>(cell.elevation * 255);
                    height = std::max(0, std::min(255, height));

                    if (height < 60)
                    {
                        // Lowlands (Grass/Forest)
                        pixels[pixelIdx] = 34 + height;          // R
                        pixels[pixelIdx + 1] = 139 + height / 2; // G
                        pixels[pixelIdx + 2] = 34;               // B
                    }
                    else if (height < 160)
                    {
                        // Mountains (Dirt/Stone)
                        int mod = height - 60;
                        pixels[pixelIdx] = 139 + mod;     // R
                        pixels[pixelIdx + 1] = 115 + mod; // G
                        pixels[pixelIdx + 2] = 85 + mod;  // B
                    }
                    else
                    {
                        // Mountain Peaks (Snow)
                        pixels[pixelIdx] = 240;     // R
                        pixels[pixelIdx + 1] = 240; // G
                        pixels[pixelIdx + 2] = 255; // B
                    }
                }
            }
        }

        std::cout << "[Step 4] Saving Tectonic Elevation Map as PPM..." << std::endl;

        // --- Draw Rivers ---
        for (const auto &corner : graph.corners)
        {
            // Only consider rendering if there is water AND it's on land
            if (corner.river > 5 && corner.downslope != -1 && corner.elevation > 0.001)
            {
                const auto &nextCorner = graph.corners[corner.downslope];

                // Double safety: Don't draw the line if the target corner is deep underwater
                // (This allows the very last segment to touch the beach, but goes no further)
                if (nextCorner.elevation >= 0.0)
                {

                    // Draw the river (Using Black to match your styling)
                    drawLine(pixels, w, h,
                             static_cast<int>(corner.position.x), static_cast<int>(corner.position.y),
                             static_cast<int>(nextCorner.position.x), static_cast<int>(nextCorner.position.y),
                             0, 0, 0); // R, G, B
                }
            }
        }

        terrain::savePPM("voronoi_map.ppm", pixels, w, h);
        std::cout << "[DONE] Successfully saved map preview to disk!" << std::endl;
    }

} // namespace terrain