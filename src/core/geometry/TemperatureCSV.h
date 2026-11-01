
#pragma once

#include <string>
#include <vector>

struct TemperatureNode {
    int nodeId = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double temperatureC = 0.0;
};

struct TemperatureDataset {
    std::vector<TemperatureNode> nodes;

    double minimumTemperature = 0.0;
    double maximumTemperature = 0.0;
};

class TemperatureCSV {
public:
    static TemperatureDataset parse(const std::string& filename);
};
