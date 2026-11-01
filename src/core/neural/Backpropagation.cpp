#include "Backpropagation.h"

#include <cmath>
#include <stdexcept>

namespace neural {

Evaluation Backpropagation::evaluate(
    const Parameters& parameters,
    double input,
    double target
) {
    const double z = parameters.w * input + parameters.b;
    const double hidden = std::tanh(z);

    const double prediction = parameters.v * hidden + parameters.c;
    const double error = prediction - target;
    const double loss = 0.5 * error * error;

    // Manual chain-rule backpropagation.
    const double outputGradient = error;
    const double hiddenGradient =
        outputGradient * parameters.v * (1.0 - hidden * hidden);

    Gradients gradients;

    gradients.v = outputGradient * hidden;
    gradients.c = outputGradient;
    gradients.w = hiddenGradient * input;
    gradients.b = hiddenGradient;

    return {
        hidden,
        prediction,
        loss,
        gradients
    };
}

double Backpropagation::calculateLoss(
    const Parameters& parameters,
    double input,
    double target
) {
    return evaluate(parameters, input, target).loss;
}

Gradients Backpropagation::finiteDifference(
    const Parameters& parameters,
    double input,
    double target,
    double epsilon
) {
    if (epsilon <= 0.0) {
        throw std::invalid_argument("Epsilon must be positive.");
    }

    Gradients numerical;

    Parameters plus = parameters;
    Parameters minus = parameters;

    plus.w += epsilon;
    minus.w -= epsilon;
    numerical.w = (
        calculateLoss(plus, input, target) -
        calculateLoss(minus, input, target)
    ) / (2.0 * epsilon);

    plus = parameters;
    minus = parameters;
    plus.b += epsilon;
    minus.b -= epsilon;
    numerical.b = (
        calculateLoss(plus, input, target) -
        calculateLoss(minus, input, target)
    ) / (2.0 * epsilon);

    plus = parameters;
    minus = parameters;
    plus.v += epsilon;
    minus.v -= epsilon;
    numerical.v = (
        calculateLoss(plus, input, target) -
        calculateLoss(minus, input, target)
    ) / (2.0 * epsilon);

    plus = parameters;
    minus = parameters;
    plus.c += epsilon;
    minus.c -= epsilon;
    numerical.c = (
        calculateLoss(plus, input, target) -
        calculateLoss(minus, input, target)
    ) / (2.0 * epsilon);

    return numerical;
}

void Backpropagation::update(
    Parameters& parameters,
    const Gradients& gradients,
    double learningRate
) {
    if (learningRate <= 0.0) {
        throw std::invalid_argument("Learning rate must be positive.");
    }

    // Calculate all new values before changing any parameter.
    const double newW = parameters.w - learningRate * gradients.w;
    const double newB = parameters.b - learningRate * gradients.b;
    const double newV = parameters.v - learningRate * gradients.v;
    const double newC = parameters.c - learningRate * gradients.c;

    parameters.w = newW;
    parameters.b = newB;
    parameters.v = newV;
    parameters.c = newC;
}

std::vector<TrainingPoint> Backpropagation::train(
    Parameters& parameters,
    double input,
    double target,
    double learningRate,
    int steps
) {
    if (steps < 0) {
        throw std::invalid_argument("Training steps cannot be negative.");
    }

    std::vector<TrainingPoint> history;
    history.reserve(static_cast<std::size_t>(steps) + 1);

    history.push_back({
        0,
        calculateLoss(parameters, input, target)
    });

    for (int step = 1; step <= steps; ++step) {
        const Evaluation result = evaluate(parameters, input, target);

        update(parameters, result.gradients, learningRate);

        history.push_back({
            step,
            calculateLoss(parameters, input, target)
        });
    }

    return history;
}

}