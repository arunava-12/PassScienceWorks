#include "Backpropagation.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void writeLossCSV(const std::vector<neural::TrainingPoint>& history) {
    std::ofstream file("results/csv/backprop_loss.csv");

    if (!file) {
        throw std::runtime_error("Could not create backprop_loss.csv");
    }

    file << "step,loss\n";
    file << std::setprecision(12);

    for (const auto& point : history) {
        file << point.step << ',' << point.loss << '\n';
    }
}

void writeLossSVG(const std::vector<neural::TrainingPoint>& history) {
    constexpr double width = 900.0;
    constexpr double height = 500.0;
    constexpr double left = 85.0;
    constexpr double right = 35.0;
    constexpr double top = 45.0;
    constexpr double bottom = 75.0;

    const double plotWidth = width - left - right;
    const double plotHeight = height - top - bottom;

    double maximumLoss = 0.0;

    for (const auto& point : history) {
        maximumLoss = std::max(maximumLoss, point.loss);
    }

    if (maximumLoss <= 0.0) {
        maximumLoss = 1.0;
    }

    std::ofstream file("results/plots/backprop_loss.svg");

    if (!file) {
        throw std::runtime_error("Could not create backprop_loss.svg");
    }

    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
         << "width=\"900\" height=\"500\" viewBox=\"0 0 900 500\">\n";

    file << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    file << "<text x=\"450\" y=\"25\" text-anchor=\"middle\" "
         << "font-size=\"20\">Backpropagation Training Loss</text>\n";

    file << "<line x1=\"" << left << "\" y1=\"" << top
         << "\" x2=\"" << left << "\" y2=\"" << top + plotHeight
         << "\" stroke=\"black\"/>\n";

    file << "<line x1=\"" << left << "\" y1=\"" << top + plotHeight
         << "\" x2=\"" << left + plotWidth << "\" y2=\""
         << top + plotHeight << "\" stroke=\"black\"/>\n";

    file << "<text x=\"450\" y=\"485\" text-anchor=\"middle\" "
         << "font-size=\"15\">Training step</text>\n";

    file << "<text x=\"20\" y=\"250\" text-anchor=\"middle\" "
         << "font-size=\"15\" transform=\"rotate(-90 20 250)\">Loss</text>\n";

    file << "<polyline fill=\"none\" stroke=\"#1769aa\" "
         << "stroke-width=\"2.5\" points=\"";

    for (const auto& point : history) {
        const double x = left +
            (static_cast<double>(point.step) / 100.0) * plotWidth;

        const double y = top + plotHeight -
            (point.loss / maximumLoss) * plotHeight;

        file << x << ',' << y << ' ';
    }

    file << "\"/>\n";

    file << "<text x=\"" << left << "\" y=\""
         << top + plotHeight + 25
         << "\" font-size=\"12\">0</text>\n";

    file << "<text x=\"" << left + plotWidth
         << "\" y=\"" << top + plotHeight + 25
         << "\" text-anchor=\"end\" font-size=\"12\">100</text>\n";

    file << "</svg>\n";
}

}

int main() {
    try {
        std::filesystem::create_directories("results/csv");
        std::filesystem::create_directories("results/plots");

        neural::Parameters parameters;

        constexpr double input = 1.0;
        constexpr double target = 2.0;
        constexpr double learningRate = 0.1;
        constexpr int trainingSteps = 100;
        constexpr double epsilon = 1e-6;

        const auto initial =
            neural::Backpropagation::evaluate(parameters, input, target);

        const auto numerical =
            neural::Backpropagation::finiteDifference(
                parameters, input, target, epsilon
            );

        const auto& analytical = initial.gradients;

        const double maxGradientError = std::max({
            std::abs(analytical.w - numerical.w),
            std::abs(analytical.b - numerical.b),
            std::abs(analytical.v - numerical.v),
            std::abs(analytical.c - numerical.c)
        });

        std::cout << std::fixed << std::setprecision(10);

        std::cout << "MANUAL BACKPROPAGATION\n";
        std::cout << "----------------------\n";
        std::cout << "Input: " << input << '\n';
        std::cout << "Target: " << target << '\n';
        std::cout << "Hidden output: " << initial.hidden << '\n';
        std::cout << "Prediction: " << initial.prediction << '\n';
        std::cout << "Initial loss: " << initial.loss << "\n\n";

        std::cout << "ANALYTICAL GRADIENTS\n";
        std::cout << "dw: " << analytical.w << '\n';
        std::cout << "db: " << analytical.b << '\n';
        std::cout << "dv: " << analytical.v << '\n';
        std::cout << "dc: " << analytical.c << "\n\n";

        std::cout << "FINITE DIFFERENCE VALIDATION\n";
        std::cout << "Maximum gradient error: " << maxGradientError << '\n';

        if (maxGradientError > 1e-5) {
            throw std::runtime_error("Gradient validation failed.");
        }

        const auto history = neural::Backpropagation::train(
            parameters,
            input,
            target,
            learningRate,
            trainingSteps
        );

        writeLossCSV(history);
        writeLossSVG(history);

        std::cout << "\nTRAINING RESULTS\n";
        std::cout << "Final loss: " << history.back().loss << '\n';
        std::cout << "Final w: " << parameters.w << '\n';
        std::cout << "Final b: " << parameters.b << '\n';
        std::cout << "Final v: " << parameters.v << '\n';
        std::cout << "Final c: " << parameters.c << '\n';

        std::cout << "\nGenerated:\n";
        std::cout << "results/csv/backprop_loss.csv\n";
        std::cout << "results/plots/backprop_loss.svg\n";

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Backpropagation error: " << error.what() << '\n';
        return 1;
    }
}