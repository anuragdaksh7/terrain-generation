# Architectural Guide

This document provides a technical overview of how the Procedural Terrain Generator works and how its components interact.

## 1. Data Model (`MapGraph`)

The core of the system is the `MapGraph` struct, which maintains a dual-graph representation:

- **Centers (`Center`)**: Represent the "cells" or "plates". They hold properties like `elevation`, `moisture`, and `isOcean`.
- **Corners (`Corner`)**: Where edges meet. These are critical for simulations that require flow (like rivers). They hold `elevation`, `moisture`, and `river` flow volume.
- **Edges (`Edge`)**: Connect two corners and border two centers. Used for river segments and tectonic boundaries.

## 2. Generation Pipeline

The generation process follows a linear pipeline in `main.cpp`:

1.  **Seed Point Generation**: Random points are scattered in a 2D plane.
2.  **Voronoi Construction**: `jc_voronoi` creates the diagram. Our engine converts this into the `MapGraph` structure, linking centers to their neighbors and corners.
3.  **Tectonic Simulation**:
    - `initializePlates`: Randomly assigns land/water status.
    - `buildMountainRanges`: Artificially increases elevation along specific paths to simulate convergence.
    - `smoothElevations`: Averages neighbor elevations to create natural gradients.
    - `assignCornerElevations`: Propagates plate elevations to the corners.
4.  **Hydrological Simulation**:
    - `calculateDownslopes`: For every corner, finds the lowest adjacent corner.
    - `simulateRivers`: Accumulates "rainfall" from peaks down to the ocean.
    - `calculateMoisture`: Determines humidity based on proximity to water and river volume.
5.  **Rendering**:
    - The `renderTectonicMap` function performs a nearest-neighbor lookup for every pixel to its closest `Center`.
    - It applies a biome shader based on `elevation` and `moisture`.
    - Rivers are drawn using a thick-brush Bresenham's algorithm.

## 3. Key Algorithms

- **Voronoi Dual-Graph**: Essential for avoiding the "grid look" of traditional noise-based terrain.
- **Bresenham's Line with Circular Brush**: Used in `Generator.cpp` to render rivers with variable thickness without needing a full vector rasterizer.
- **Fluvial Accumulation**: A simple but effective way to model watershed systems.

## 4. Extending the Engine

To add new features:
- **New Biomes**: Update the "Biome Shader" section in `renderTectonicMap` in `Generator.cpp`.
- **New Simulations**: Add a header in `include/terrain/Simulation/` and implement the logic using the `MapGraph` structure.
- **Alternative IO**: Implement new functions in `ImageExport.cpp` (e.g., using `stb_image_write.h` for PNG support).
