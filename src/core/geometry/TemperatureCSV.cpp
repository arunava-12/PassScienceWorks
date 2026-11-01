
#include "TemperatureCSV.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace {

std::string trim(const std::string& value) {
    const std::size_t first = value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = value.find_last_not_of(" \t\r\n");

    return value.substr(first, last - first + 1);
}

double parseFiniteDouble(const std::string& value) {
    std::size_t parsedCharacters = 0;
    double result = std::stod(value, &parsedCharacters);

    if (parsedCharacters != value.size() || !std::isfinite(result)) {
        throw std::runtime_error("Invalid or non-finite numeric value.");
    }

    return result;
}

int parseNodeId(const std::string& value) {
    std::size_t parsedCharacters = 0;
    int result = std::stoi(value, &parsedCharacters);

    if (parsedCharacters != value.size() || result <= 0) {
        throw std::runtime_error("Invalid node_id.");
    }

    return result;
}

}

TemperatureDataset TemperatureCSV::parse(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open CSV: " + filename);
    }

    std::string line;

    if (!std::getline(file, line)) {
        throw std::runtime_error("CSV file is empty.");
    }

    const std::string expectedHeader =
        "node_id,x_m,y_m,z_m,temperature_C";

    if (trim(line) != expectedHeader) {
        throw std::runtime_error(
            "Invalid CSV header. Expected: " + expectedHeader
        );
    }

    TemperatureDataset dataset;
    std::unordered_set<int> nodeIds;

    bool firstNode = true;
    std::size_t lineNumber = 1;

    while (std::getline(file, line)) {
        lineNumber++;

        if (trim(line).empty()) {
            continue;
        }

        if (dataset.nodes.size() >= 2000000) {
            throw std::runtime_error(
                "CSV exceeds the 2,000,000 node limit."
            );
        }

        std::stringstream stream(line);
        std::string fields[5];

        for (int i = 0; i < 5; i++) {
            if (!std::getline(stream, fields[i], ',')) {
                throw std::runtime_error(
                    "Missing CSV field at line " +
                    std::to_string(lineNumber)
                );
            }

            fields[i] = trim(fields[i]);
        }

        std::string extra;

        if (std::getline(stream, extra, ',')) {
            throw std::runtime_error(
                "Too many CSV fields at line " +
                std::to_string(lineNumber)
            );
        }

        try {
            TemperatureNode node;

            node.nodeId = parseNodeId(fields[0]);
            node.x = parseFiniteDouble(fields[1]);
            node.y = parseFiniteDouble(fields[2]);
            node.z = parseFiniteDouble(fields[3]);
            node.temperatureC = parseFiniteDouble(fields[4]);

            if (!nodeIds.insert(node.nodeId).second) {
                throw std::runtime_error("Duplicate node_id.");
            }

            if (firstNode) {
                dataset.minimumTemperature = node.temperatureC;
                dataset.maximumTemperature = node.temperatureC;
                firstNode = false;
            }
            else {
                dataset.minimumTemperature = std::min(
                    dataset.minimumTemperature,
                    node.temperatureC
                );

                dataset.maximumTemperature = std::max(
                    dataset.maximumTemperature,
                    node.temperatureC
                );
            }

            dataset.nodes.push_back(node);
        }
        catch (const std::exception& error) {
            throw std::runtime_error(
                "CSV line " + std::to_string(lineNumber) +
                ": " + error.what()
            );
        }
    }

    if (dataset.nodes.empty()) {
        throw std::runtime_error("CSV contains no temperature nodes.");
    }

    return dataset;
}
