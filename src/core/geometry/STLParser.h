
#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Triangle {
    Vec3 normal;
    Vec3 vertices[3];
};

struct STLMesh {
    std::string name;
    std::vector<Triangle> triangles;

    std::size_t vertexCount() const {
        return triangles.size() * 3;
    }
};

class STLParser {
public:
    static STLMesh parseASCII(const std::string& filename);
};
