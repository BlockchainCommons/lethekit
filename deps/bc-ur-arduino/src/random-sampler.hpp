//
//  random-sampler.hpp
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef BC_UR_RANDOM_SAMPLER22_HPP
#define BC_UR_RANDOM_SAMPLER22_HPP

#include <ArduinoSTL.h>
#include <vector>
#include <functional>
#include <Arduino.h>

// Random-number sampling using the Walker-Vose alias method,
// as described by Keith Schwarz (2011)
// http://www.keithschwarz.com/darts-dice-coins

// Based on C implementation:
// https://jugit.fz-juelich.de/mlz/ransampl

// Translated to C++ by Wolf McNally

namespace ur_arduino {

class RandomSampler final {
public:
    RandomSampler(std::vector<double> probs);
    int next(double r1, double r2);

private:
    std::vector<double> probs_;
    std::vector<int> aliases_;
};

}

#endif // BC_UR_RANDOM_SAMPLER_HPP
