#include "Backpropagation.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testInitialForwardPass() {
    neural::Parameters parameters;

    const auto result =
        neural::Backpropagation::evaluate(parameters, 1.0, 2.0);

    require(
        std::abs(result.hidden - std::tanh(0.5)) < 1e-12,
        "Incorrect hidden-layer output."
    );

    require(
        std::abs(result.prediction - std::tanh(0.5)) < 1e-12,
        "Incorrect prediction."
    );

    require(result.loss > 0.0, "Initial loss should be positive.");
}

void testFiniteDifferenceGradients() {
    neural::Parameters parameters;

    const auto analytical =
        neural::Backpropagation::evaluate(parameters, 1.0, 2.0).gradients;

    const auto numerical =
        neural::Backpropagation::finiteDifference(
            parameters, 1.0, 2.0, 1e-6
        );

    require(
        std::abs(analytical.w - numerical.w) <= 1e-5,
        "Weight gradient failed finite-difference validation."
    );

    require(
        std::abs(analytical.b - numerical.b) <= 1e-5,
        "Bias gradient failed finite-difference validation."
    );

    require(
        std::abs(analytical.v - numerical.v) <= 1e-5,
        "Output-weight gradient failed finite-difference validation."
    );

    require(
        std::abs(analytical.c - numerical.c) <= 1e-5,
        "Output-bias gradient failed finite-difference validation."
    );
}

void testTrainingReducesLoss() {
    neural::Parameters parameters;

    const double initialLoss =
        neural::Backpropagation::calculateLoss(parameters, 1.0, 2.0);

    const auto history =
        neural::Backpropagation::train(parameters, 1.0, 2.0, 0.1, 100);

    const double finalLoss =
        neural::Backpropagation::calculateLoss(parameters, 1.0, 2.0);

    require(history.size() == 101, "Expected 101 loss measurements.");
    require(finalLoss < initialLoss, "Training did not reduce the loss.");
}

}

int main() {
    try {
        testInitialForwardPass();
        testFiniteDifferenceGradients();
        testTrainingReducesLoss();

        std::cout << "All neural network tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Neural test failed: " << error.what() << '\n';
        return 1;
    }
}