
#include "core/thermal/ThermalSolver.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    try {
        std::filesystem::create_directories("results/csv");

        const std::vector<int> meshes = {10, 20, 40, 80};

        std::cout << "\nPASS SCIENCE WORKS - THERMAL FEM SOLVER\n";
        std::cout << "----------------------------------------\n";

        std::cout << std::left
                  << std::setw(12) << "Elements"
                  << std::setw(20) << "Max Error (C)"
                  << "\n";

        for (int elements : meshes) {
            ThermalParameters parameters;
            parameters.elements = elements;

            ThermalResult result = ThermalSolver::solve(parameters);

            const std::string filename =
                "results/csv/thermal_" +
                std::to_string(elements) + ".csv";

            ThermalSolver::exportCSV(result, filename);

            std::cout << std::left
                      << std::setw(12) << elements
                      << std::scientific << std::setprecision(6)
                      << result.maximumMidpointError
                      << "\n";

            std::cout << "Exported: " << filename << "\n";
        }

        std::cout << "\nThermal FEM computation completed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << "\n";
        return 1;
    }
}
