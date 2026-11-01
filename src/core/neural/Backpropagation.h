#pragma once

#include <array>
#include <vector>

namespace neural {

struct Parameters {
    double w = 0.5;
    double b = 0.0;
    double v = 1.0;
    double c = 0.0;
};

struct Gradients {
    double w = 0.0;
    double b = 0.0;
    double v = 0.0;
    double c = 0.0;
};

struct Evaluation {
    double hidden = 0.0;
    double prediction = 0.0;
    double loss = 0.0;
    Gradients gradients;
};

struct TrainingPoint {
    int step = 0;
    double loss = 0.0;
};

class Backpropagation {
public:
    static Evaluation evaluate(
        const Parameters& parameters,
        double input,
        double target
    );

    static double calculateLoss(
        const Parameters& parameters,
        double input,
        double target
    );

    static Gradients finiteDifference(
        const Parameters& parameters,
        double input,
        double target,
        double epsilon
    );

    static void update(
        Parameters& parameters,
        const Gradients& gradients,
        double learningRate
    );

    static std::vector<TrainingPoint> train(
        Parameters& parameters,
        double input,
        double target,
        double learningRate,
        int steps
    );
};

}