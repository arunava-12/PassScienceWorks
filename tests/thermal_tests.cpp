
#include "ThermalSolver.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testBoundaryConditions() {
    ThermalParameters p;
    p.elements = 20;

    ThermalResult result = ThermalSolver::solve(p);

    require(
        std::abs(result.temperatures.front() - 100.0) < 1.0e-10,
        "Left boundary temperature is incorrect."
    );

    require(
        std::abs(result.temperatures.back() - 20.0) < 1.0e-10,
        "Right boundary temperature is incorrect."
    );

    std::cout << "[PASS] Boundary conditions\n";
}

void testMidpointError() {
    ThermalParameters p;
    p.elements = 10;

    ThermalResult result = ThermalSolver::solve(p);

    require(
        std::abs(result.maximumMidpointError - 1.25) < 1.0e-8,
        "Incorrect midpoint error for 10 elements."
    );

    std::cout << "[PASS] Midpoint error calculation\n";
}

void testMeshConvergence() {
    const int meshes[] = {10, 20, 40, 80};

    double previousError = 0.0;

    for (int elements : meshes) {
        ThermalParameters p;
        p.elements = elements;

        ThermalResult result = ThermalSolver::solve(p);

        if (previousError > 0.0) {
            require(
                result.maximumMidpointError < previousError,
                "Mesh refinement did not reduce error."
            );

            const double ratio =
                previousError / result.maximumMidpointError;

            require(
                std::abs(ratio - 4.0) < 1.0e-6,
                "Unexpected convergence ratio."
            );
        }

        previousError = result.maximumMidpointError;
    }

    std::cout << "[PASS] Mesh convergence\n";
}

void testInvalidParameters() {
    ThermalParameters p;
    p.conductivity = 0.0;

    bool exceptionThrown = false;

    try {
        ThermalSolver::solve(p);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    require(
        exceptionThrown,
        "Invalid conductivity was not rejected."
    );

    std::cout << "[PASS] Invalid parameter validation\n";
}

int main() {
    try {
        std::cout << "\nTHERMAL FEM AUTOMATED TESTS\n";
        std::cout << "---------------------------\n";

        testBoundaryConditions();
        testMidpointError();
        testMeshConvergence();
        testInvalidParameters();

        std::cout << "\nAll thermal tests passed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "\n[FAIL] " << error.what() << "\n";
        return 1;
    }
}
