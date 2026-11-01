
#include "ThermalSolver.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace {

std::vector<double> solveLinearSystem(
    std::vector<std::vector<double>> matrix,
    std::vector<double> rhs
) {
    const int n = static_cast<int>(rhs.size());

    // Gaussian elimination with partial pivoting.
    for (int column = 0; column < n; column++) {
        int pivot = column;

        for (int row = column + 1; row < n; row++) {
            if (std::abs(matrix[row][column]) >
                std::abs(matrix[pivot][column])) {
                pivot = row;
            }
        }

        if (std::abs(matrix[pivot][column]) < 1.0e-14) {
            throw std::runtime_error("Singular FEM stiffness matrix.");
        }

        std::swap(matrix[column], matrix[pivot]);
        std::swap(rhs[column], rhs[pivot]);

        for (int row = column + 1; row < n; row++) {
            double factor = matrix[row][column] /
                            matrix[column][column];

            for (int j = column; j < n; j++) {
                matrix[row][j] -= factor * matrix[column][j];
            }

            rhs[row] -= factor * rhs[column];
        }
    }

    std::vector<double> solution(n);

    for (int row = n - 1; row >= 0; row--) {
        double value = rhs[row];

        for (int j = row + 1; j < n; j++) {
            value -= matrix[row][j] * solution[j];
        }

        solution[row] = value / matrix[row][row];
    }

    return solution;
}

void validate(const ThermalParameters& p) {
    if (!std::isfinite(p.length) ||
        !std::isfinite(p.area) ||
        !std::isfinite(p.conductivity) ||
        !std::isfinite(p.heatGeneration) ||
        !std::isfinite(p.leftTemperature) ||
        !std::isfinite(p.rightTemperature)) {
        throw std::invalid_argument("Thermal parameters must be finite.");
    }

    if (p.length <= 0.0 ||
        p.area <= 0.0 ||
        p.conductivity <= 0.0 ||
        p.elements <= 0 ||
        p.elements > 10000) {
        throw std::invalid_argument("Invalid thermal model dimensions.");
    }
}

}

double ThermalSolver::exactTemperature(
    double x,
    const ThermalParameters& p
) {
    const double linear =
        p.leftTemperature +
        (p.rightTemperature - p.leftTemperature) * x / p.length;

    const double source =
        (p.heatGeneration / (2.0 * p.conductivity)) *
        x * (p.length - x);

    return linear + source;
}

ThermalResult ThermalSolver::solve(const ThermalParameters& p) {
    validate(p);

    const int nodeCount = p.elements + 1;
    const double h = p.length / p.elements;

    std::vector<std::vector<double>> stiffness(
        nodeCount, std::vector<double>(nodeCount, 0.0)
    );

    std::vector<double> force(nodeCount, 0.0);

    // Assemble element stiffness matrices and source vectors.
    for (int element = 0; element < p.elements; element++) {
        const double elementStiffness =
            p.conductivity * p.area / h;

        const double elementForce =
            p.heatGeneration * p.area * h / 2.0;

        const int left = element;
        const int right = element + 1;

        stiffness[left][left] += elementStiffness;
        stiffness[left][right] -= elementStiffness;
        stiffness[right][left] -= elementStiffness;
        stiffness[right][right] += elementStiffness;

        force[left] += elementForce;
        force[right] += elementForce;
    }

    // Apply Dirichlet boundary conditions.
    const double leftTemperature = p.leftTemperature;
    const double rightTemperature = p.rightTemperature;

    for (int row = 0; row < nodeCount; row++) {
        if (row != 0) {
            force[row] -= stiffness[row][0] * leftTemperature;
        }

        if (row != nodeCount - 1) {
            force[row] -=
                stiffness[row][nodeCount - 1] * rightTemperature;
        }
    }

    for (int i = 0; i < nodeCount; i++) {
        stiffness[0][i] = 0.0;
        stiffness[i][0] = 0.0;

        stiffness[nodeCount - 1][i] = 0.0;
        stiffness[i][nodeCount - 1] = 0.0;
    }

    stiffness[0][0] = 1.0;
    stiffness[nodeCount - 1][nodeCount - 1] = 1.0;

    force[0] = leftTemperature;
    force[nodeCount - 1] = rightTemperature;

    const std::vector<double> temperatures =
        solveLinearSystem(stiffness, force);

    ThermalResult result;

    result.positions.resize(nodeCount);
    result.temperatures = temperatures;

    for (int i = 0; i < nodeCount; i++) {
        result.positions[i] = i * h;
    }

    // Compare the FE interpolation with the exact solution
    // at the midpoint of every element.
    result.maximumMidpointError = 0.0;

    for (int element = 0; element < p.elements; element++) {
        const double midpoint = (element + 0.5) * h;

        const double finiteElementTemperature =
            0.5 * (temperatures[element] + temperatures[element + 1]);

        const double exact =
            exactTemperature(midpoint, p);

        const double error =
            std::abs(exact - finiteElementTemperature);

        result.maximumMidpointError =
            std::max(result.maximumMidpointError, error);
    }

    return result;
}

void ThermalSolver::exportCSV(
    const ThermalResult& result,
    const std::string& filename
) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to create CSV file: " + filename);
    }

    file << "position_m,temperature_C\n";
    file << std::fixed << std::setprecision(10);

    for (std::size_t i = 0; i < result.positions.size(); i++) {
        file << result.positions[i] << ","
             << result.temperatures[i] << "\n";
    }
}
