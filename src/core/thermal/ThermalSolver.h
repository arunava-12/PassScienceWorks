
#pragma once

#include <string>
#include <vector>

struct ThermalParameters {
    double length = 1.0;
    double area = 1.0e-4;
    double conductivity = 10.0;
    double heatGeneration = 10000.0;
    double leftTemperature = 100.0;
    double rightTemperature = 20.0;
    int elements = 10;
};

struct ThermalResult {
    std::vector<double> positions;
    std::vector<double> temperatures;
    double maximumMidpointError = 0.0;
};

class ThermalSolver {
public:
    static ThermalResult solve(const ThermalParameters& parameters);

    static double exactTemperature(
        double x,
        const ThermalParameters& parameters
    );

    static void exportCSV(
        const ThermalResult& result,
        const std::string& filename
    );
};
