
#include "core/performance/Benchmark.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        BenchmarkConfig config;

        config.seed = 20261101;
        config.numberOfCases = 128;
        config.minimumElements = 100;
        config.maximumElements = 150;
        config.repetitions = 5;
        config.warmupRuns = 1;
        config.maximumWorkers = 4;

        std::cout << "\nPASS SCIENCE WORKS - PERFORMANCE BENCHMARK\n";
        std::cout << "-------------------------------------------\n";

        std::cout << "Generating deterministic workload...\n";

        BenchmarkReport report = Benchmark::run(config);

        std::filesystem::create_directories("results/csv");

        Benchmark::exportCSV(
            report,
            "results/csv/performance_benchmark.csv"
        );

        std::cout << "\nHardware and workload information\n";
        std::cout << "Logical CPUs: " << report.logicalCpuCount << "\n";
        std::cout << "Cases: " << report.numberOfCases << "\n";
        std::cout << "Random seed: " << report.seed << "\n";
        std::cout << "Measured repetitions: "
                  << report.repetitions << "\n";
        std::cout << "Maximum tested workers: "
                  << report.maximumTestedWorkers << "\n";

        std::cout << "\nBenchmark results\n";

        std::cout << std::left
                  << std::setw(12) << "Workers"
                  << std::setw(18) << "Median (ms)"
                  << std::setw(14) << "Speedup"
                  << std::setw(14) << "Efficiency"
                  << "Max Difference (C)\n";

        std::cout << std::fixed << std::setprecision(6);

        for (const WorkerBenchmarkResult& result : report.results) {
            std::cout << std::left
                      << std::setw(12) << result.workers
                      << std::setw(18) << result.medianMilliseconds
                      << std::setw(14) << result.speedup
                      << std::setw(14) << result.efficiency
                      << result.maximumTemperatureDifference
                      << "\n";
        }

        std::cout << "\nCSV exported to "
                  << "results/csv/performance_benchmark.csv\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "\nBENCHMARK ERROR: "
                  << error.what() << "\n";

        return 1;
    }
}
