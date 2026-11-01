
#include "Benchmark.h"

#include "ThermalSolver.h"
#include "WorkerPool.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

std::vector<ThermalResult> solveSerial(
    const std::vector<ThermalCase>& workload
) {
    std::vector<ThermalResult> results(workload.size());

    for (std::size_t i = 0; i < workload.size(); i++) {
        ThermalParameters parameters;

        parameters.conductivity = workload[i].conductivity;
        parameters.heatGeneration = workload[i].heatGeneration;
        parameters.elements = workload[i].elements;

        results[i] = ThermalSolver::solve(parameters);
    }

    return results;
}

std::vector<ThermalResult> solveParallel(
    const std::vector<ThermalCase>& workload,
    unsigned int workers
) {
    std::vector<ThermalResult> results(workload.size());

    WorkerPool::parallelFor(
        workload.size(),
        workers,
        [&](std::size_t i) {
            ThermalParameters parameters;

            parameters.conductivity = workload[i].conductivity;
            parameters.heatGeneration = workload[i].heatGeneration;
            parameters.elements = workload[i].elements;

            results[i] = ThermalSolver::solve(parameters);
        }
    );

    return results;
}

double elapsedMilliseconds(Clock::time_point start) {
    const auto end = Clock::now();

    return std::chrono::duration<double, std::milli>(
        end - start
    ).count();
}

double calculateMedian(std::vector<double> values) {
    if (values.empty()) {
        throw std::runtime_error("Cannot calculate median of empty data.");
    }

    std::sort(values.begin(), values.end());

    const std::size_t middle = values.size() / 2;

    if (values.size() % 2 == 0) {
        return (values[middle - 1] + values[middle]) / 2.0;
    }

    return values[middle];
}

double maximumDifference(
    const std::vector<ThermalResult>& reference,
    const std::vector<ThermalResult>& candidate
) {
    if (reference.size() != candidate.size()) {
        throw std::runtime_error("Benchmark result counts do not match.");
    }

    double maximumError = 0.0;

    for (std::size_t i = 0; i < reference.size(); i++) {
        if (reference[i].temperatures.size() !=
            candidate[i].temperatures.size()) {
            throw std::runtime_error("Temperature vector sizes do not match.");
        }

        for (std::size_t j = 0;
             j < reference[i].temperatures.size();
             j++) {

            const double difference = std::abs(
                reference[i].temperatures[j] -
                candidate[i].temperatures[j]
            );

            maximumError = std::max(maximumError, difference);
        }
    }

    return maximumError;
}

}

std::vector<ThermalCase> Benchmark::generateWorkload(
    const BenchmarkConfig& config
) {
    if (config.numberOfCases <= 0 ||
        config.minimumElements <= 0 ||
        config.maximumElements < config.minimumElements ||
        config.maximumElements > 10000) {
        throw std::invalid_argument("Invalid benchmark workload settings.");
    }

    std::mt19937 generator(config.seed);

    std::uniform_real_distribution<double> conductivityDistribution(
        5.0, 50.0
    );

    std::uniform_real_distribution<double> heatDistribution(
        1000.0, 20000.0
    );

    std::uniform_int_distribution<int> elementDistribution(
        config.minimumElements,
        config.maximumElements
    );

    std::vector<ThermalCase> workload;
    workload.reserve(config.numberOfCases);

    for (int i = 0; i < config.numberOfCases; i++) {
        ThermalCase thermalCase;

        thermalCase.conductivity = conductivityDistribution(generator);
        thermalCase.heatGeneration = heatDistribution(generator);
        thermalCase.elements = elementDistribution(generator);

        workload.push_back(thermalCase);
    }

    return workload;
}

BenchmarkReport Benchmark::run(const BenchmarkConfig& config) {
    if (config.repetitions < 1 || config.warmupRuns < 0) {
        throw std::invalid_argument("Invalid benchmark repetition settings.");
    }

    if (config.maximumWorkers < 1) {
        throw std::invalid_argument("At least one worker is required.");
    }

    const std::vector<ThermalCase> workload =
        generateWorkload(config);

    BenchmarkReport report;

    report.logicalCpuCount = std::max(
        1U,
        std::thread::hardware_concurrency()
    );

    report.maximumTestedWorkers = std::min(
        config.maximumWorkers,
        report.logicalCpuCount
    );

    report.numberOfCases = config.numberOfCases;
    report.repetitions = config.repetitions;
    report.seed = config.seed;

    // Warm-up serial execution.
    for (int i = 0; i < config.warmupRuns; i++) {
        solveSerial(workload);
    }

    // Measure serial execution.
    std::vector<double> serialTimes;
    std::vector<ThermalResult> referenceResults;

    for (int repetition = 0;
         repetition < config.repetitions;
         repetition++) {

        const auto start = Clock::now();

        std::vector<ThermalResult> results =
            solveSerial(workload);

        serialTimes.push_back(elapsedMilliseconds(start));

        referenceResults = std::move(results);
    }

    const double serialMedian = calculateMedian(serialTimes);

    WorkerBenchmarkResult serialResult;

    serialResult.workers = 1;
    serialResult.medianMilliseconds = serialMedian;
    serialResult.speedup = 1.0;
    serialResult.efficiency = 1.0;
    serialResult.maximumTemperatureDifference = 0.0;

    report.results.push_back(serialResult);

    // Measure parallel execution using 2, 3 and 4 workers,
    // capped by the machine's logical CPU count.
    for (unsigned int workers = 2;
         workers <= report.maximumTestedWorkers;
         workers++) {

        for (int i = 0; i < config.warmupRuns; i++) {
            solveParallel(workload, workers);
        }

        std::vector<double> parallelTimes;
        double maximumError = 0.0;

        for (int repetition = 0;
             repetition < config.repetitions;
             repetition++) {

            const auto start = Clock::now();

            std::vector<ThermalResult> results =
                solveParallel(workload, workers);

            parallelTimes.push_back(elapsedMilliseconds(start));

            maximumError = std::max(
                maximumError,
                maximumDifference(referenceResults, results)
            );
        }

        const double parallelMedian =
            calculateMedian(parallelTimes);

        WorkerBenchmarkResult result;

        result.workers = workers;
        result.medianMilliseconds = parallelMedian;
        result.speedup = serialMedian / parallelMedian;
        result.efficiency = result.speedup / workers;
        result.maximumTemperatureDifference = maximumError;

        if (maximumError > 1.0e-8) {
            throw std::runtime_error(
                "Serial and parallel numerical results differ "
                "by more than 1e-8 degrees Celsius."
            );
        }

        report.results.push_back(result);
    }

    return report;
}

void Benchmark::exportCSV(
    const BenchmarkReport& report,
    const std::string& filename
) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Unable to create benchmark CSV: " + filename
        );
    }

    file << "workers,median_ms,speedup,efficiency,"
            "max_temperature_difference_C\n";

    file << std::fixed << std::setprecision(10);

    for (const WorkerBenchmarkResult& result : report.results) {
        file << result.workers << ","
             << result.medianMilliseconds << ","
             << result.speedup << ","
             << result.efficiency << ","
             << result.maximumTemperatureDifference << "\n";
    }
}
