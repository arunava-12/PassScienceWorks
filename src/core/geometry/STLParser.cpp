
#include "STLParser.h"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

bool isFinite(const Vec3& value) {
    return std::isfinite(value.x) &&
           std::isfinite(value.y) &&
           std::isfinite(value.z);
}

}

STLMesh STLParser::parseASCII(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open STL file: " + filename);
    }

    STLMesh mesh;
    mesh.name = filename;

    std::string token;
    Triangle currentTriangle;
    int vertexIndex = 0;

    while (file >> token) {
        if (token == "facet") {
            std::string normalToken;

            if (!(file >> normalToken) || normalToken != "normal") {
                throw std::runtime_error("Malformed STL facet normal.");
            }

            if (!(file >> currentTriangle.normal.x
                      >> currentTriangle.normal.y
                      >> currentTriangle.normal.z)) {
                throw std::runtime_error("Incomplete STL normal.");
            }

            if (!isFinite(currentTriangle.normal)) {
                throw std::runtime_error("STL contains a non-finite normal.");
            }

            vertexIndex = 0;
        }
        else if (token == "vertex") {
            if (vertexIndex >= 3) {
                throw std::runtime_error(
                    "STL facet contains more than three vertices."
                );
            }

            Vec3 vertex;

            if (!(file >> vertex.x >> vertex.y >> vertex.z)) {
                throw std::runtime_error("Incomplete STL vertex.");
            }

            if (!isFinite(vertex)) {
                throw std::runtime_error(
                    "STL contains a non-finite vertex."
                );
            }

            currentTriangle.vertices[vertexIndex] = vertex;
            vertexIndex++;
        }
        else if (token == "endfacet") {
            if (vertexIndex != 3) {
                throw std::runtime_error(
                    "STL facet must contain exactly three vertices."
                );
            }

            mesh.triangles.push_back(currentTriangle);

            if (mesh.triangles.size() > 2000000) {
                throw std::runtime_error(
                    "STL exceeds the 2,000,000 triangle limit."
                );
            }

            vertexIndex = 0;
        }
    }

    if (mesh.triangles.empty()) {
        throw std::runtime_error(
            "No triangles found. Only ASCII STL is supported."
        );
    }

    return mesh;
}
