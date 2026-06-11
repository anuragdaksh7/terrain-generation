#include "terrain/IO/ImageExport.hpp"
#include <fstream>
#include <iostream>

namespace terrain {

    void savePPM(const std::string& filename, const std::vector<unsigned char>& pixels, int width, int height) {
        std::ofstream file(filename, std::ios::binary);
        
        if (!file.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing!" << std::endl;
            return;
        }

        file << "P6\n" << width << " " << height << "\n255\n";
        
        file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
        file.close();
    }

}