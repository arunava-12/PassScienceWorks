
#include "STLParser.h"
#include "TemperatureCSV.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void createTestSTL(const std::string& filename) {
    std::ofstream file(filename);

    file <<
        "solid test\n"
        "facet normal 0 0 1\n"
        "outer loop\n"
        "vertex 0 0 0\n"
        "vertex 1 0 0\n"
        "vertex 0 1 0\n"
        "endloop\n"
        "endfacet\n"
        "endsolid test\n";
}

void createTestCSV(const std::string& filename) {
    std::ofstream file(filename);

    file <<
        "node_id,x_m,y_m,z_m,temperature_C\n"
        "1,0.0,0.0,0.0,100.0\n"
        "2,0.5,0.0,0.0,60.0\n"
        "3,1.0,0.0,0.0,20.0\n";
}

void testSTLParser() {
    const std::string filename = "test_geometry.stl";
    createTestSTL(filename);

    STLMesh mesh = STLParser::parseASCII(filename);

    require(mesh.triangles.size() == 1, "Incorrect triangle count.");
    require(mesh.vertexCount() == 3, "Incorrect vertex count.");
    require(
        std::abs(mesh.triangles[0].vertices[1].x - 1.0) < 1.0e-12,
        "Incorrect vertex coordinate."
    );

    std::filesystem::remove(filename);

    std::cout << "[PASS] ASCII STL parsing\n";
}

void testTemperatureCSV() {
    const std::string filename = "test_temperature.csv";
    createTestCSV(filename);

    TemperatureDataset dataset = TemperatureCSV::parse(filename);

    require(dataset.nodes.size() == 3, "Incorrect node count.");
    require(
        std::abs(dataset.minimumTemperature - 20.0) < 1.0e-12,
        "Incorrect minimum temperature."
    );
    require(
        std::abs(dataset.maximumTemperature - 100.0) < 1.0e-12,
        "Incorrect maximum temperature."
    );

    std::filesystem::remove(filename);

    std::cout << "[PASS] Temperature CSV parsing\n";
}

void testDuplicateNodeRejection() {
    const std::string filename = "test_duplicate.csv";

    {
        std::ofstream file(filename);

        file <<
            "node_id,x_m,y_m,z_m,temperature_C\n"
            "1,0,0,0,100\n"
            "1,1,0,0,90\n";
    }

    bool rejected = false;

    try {
        TemperatureCSV::parse(filename);
    }
    catch (const std::runtime_error&) {
        rejected = true;
    }

    std::filesystem::remove(filename);

    require(rejected, "Duplicate node IDs were not rejected.");

    std::cout << "[PASS] Duplicate node validation\n";
}

void testNonFiniteTemperatureRejection() {
    const std::string filename = "test_invalid.csv";

    {
        std::ofstream file(filename);

        file <<
            "node_id,x_m,y_m,z_m,temperature_C\n"
            "1,0,0,0,nan\n";
    }

    bool rejected = false;

    try {
        TemperatureCSV::parse(filename);
    }
    catch (const std::runtime_error&) {
        rejected = true;
    }

    std::filesystem::remove(filename);

    require(rejected, "Non-finite temperature was not rejected.");

    std::cout << "[PASS] Non-finite value validation\n";
}

int main() {
    try {
        std::cout << "\nGEOMETRY AND CSV TESTS\n";
        std::cout << "----------------------\n";

        testSTLParser();
        testTemperatureCSV();
        testDuplicateNodeRejection();
        testNonFiniteTemperatureRejection();

        std::cout << "\nAll geometry tests passed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "\n[FAIL] " << error.what() << "\n";
        return 1;
    }
}
