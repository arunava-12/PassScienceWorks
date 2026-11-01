
#include "Benchmark.h"
#include "WorkerPool.h"

#include <atomic>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testDeterministicWorkload() {
    BenchmarkConfig config;
    config.seed = 12345;
    config.numberOfCases = 10;

    const auto first = Benchmark::generateWorkload(config);
    const auto second = Benchmark::generateWorkload(config);

    require(first.size() == second.size(), "Workload sizes differ.");

    for (std::size_t i = 0; i < first.size(); i++) {
        require(
            first[i].conductivity == second[i].conductivity &&
            first[i].heatGeneration == second[i].heatGeneration &&
            first[i].elements == second[i].elements,
            "Fixed-seed workloads are not identical."
        );

        require(first[i].conductivity > 0.0,
                "Conductivity must be positive.");

        require(first[i].heatGeneration > 0.0,
                "Heat generation must be positive.");
    }

    std::cout << "[PASS] Fixed-seed deterministic workload\n";
}

void testWorkerPoolExecution() {
    const std::size_t taskCount = 100;
    std::vector<int> results(taskCount, 0);

    WorkerPool::parallelFor(
        taskCount,
        4,
        [&](std::size_t index) {
            results[index] = static_cast<int>(index + 1);
        }
    );

    for (std::size_t i = 0; i < taskCount; i++) {
        require(
            results[i] == static_cast<int>(i + 1),
            "Worker pool lost or misplaced a task result."
        );
    }

    std::cout << "[PASS] Parallel task execution and result ordering\n";
}

void testWorkerPoolExceptionPropagation() {
    bool exceptionCaught = false;

    try {
        WorkerPool::parallelFor(
            20,
            4,
            [](std::size_t index) {
                if (index == 5) {
                    throw std::runtime_error("Test worker exception.");
                }
            }
        );
    }
    catch (const std::runtime_error&) {
        exceptionCaught = true;
    }

    require(
        exceptionCaught,
        "Worker pool did not propagate a worker exception."
    );

    std::cout << "[PASS] Worker exception propagation\n";
}

void testBenchmarkConsistency() {
    BenchmarkConfig config;

    config.seed = 20261101;
    config.numberOfCases = 6;
    config.minimumElements = 10;
    config.maximumElements = 15;
    config.repetitions = 2;
    config.warmupRuns = 1;
    config.maximumWorkers = 2;

    BenchmarkReport report = Benchmark::run(config);

    require(!report.results.empty(), "Benchmark returned no results.");

    require(
        report.results.front().workers == 1,
        "Serial baseline must be the first result."
    );

    for (const WorkerBenchmarkResult& result : report.results) {
        require(
            result.medianMilliseconds > 0.0,
            "Benchmark duration must be positive."
        );

        require(
            result.maximumTemperatureDifference <= 1.0e-8,
            "Serial and parallel results differ."
        );
    }

    std::cout << "[PASS] Serial-parallel numerical consistency\n";
}

int main() {
    try {
        std::cout << "\nPERFORMANCE MODULE TESTS\n";
        std::cout << "------------------------\n";

        testDeterministicWorkload();
        testWorkerPoolExecution();
        testWorkerPoolExceptionPropagation();
        testBenchmarkConsistency();

        std::cout << "\nAll performance tests passed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "\n[FAIL] " << error.what() << "\n";
        return 1;
    }
}
