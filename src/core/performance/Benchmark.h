
#pragma once

#include <string>
#include <vector>

struct BenchmarkConfig {
    unsigned int seed = 20261101;
    int numberOfCases = 32;
    int minimumElements = 60;
    int maximumElements = 100;
    int repetitions = 5;
    int warmupRuns = 1;
    unsigned int maximumWorkers = 4;
};

struct ThermalCase {
    double conductivity = 10.0;
    double heatGeneration = 10000.0;
    int elements = 80;
};

struct WorkerBenchmarkResult {
    unsigned int workers = 1;
    double medianMilliseconds = 0.0;
    double speedup = 1.0;
    double efficiency = 1.0;
    double maximumTemperatureDifference = 0.0;
};

struct BenchmarkReport {
    unsigned int logicalCpuCount = 1;
    unsigned int maximumTestedWorkers = 1;

    int numberOfCases = 0;
    int repetitions = 0;
    unsigned int seed = 0;

    std::vector<WorkerBenchmarkResult> results;
};

class Benchmark {
public:
    static std::vector<ThermalCase> generateWorkload(
        const BenchmarkConfig& config
    );

    static BenchmarkReport run(
        const BenchmarkConfig& config
    );

    static void exportCSV(
        const BenchmarkReport& report,
        const std::string& filename
    );
};
