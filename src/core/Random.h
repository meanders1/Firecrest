#pragma once

#include <random>

namespace fc {

class Random {
private:
    std::mt19937 _rng;
    static std::mt19937 _staticRNG;

public:
    Random();
    Random(unsigned int seed);
    void seed(unsigned int seed);
    int nextInt();
    unsigned int nextUInt();
    float nextFraction();
    // non-inclusive
    int nextInt(int min, int max);
    // Inclusive
    int nextIntInc(int min, int max);
    // non-inclusive
    float nextFloat(float min, float max);
    // Inclusive
    float nextFloatInc(float min, float max);

public:
    // Returns a random number in the range [0, 1>
    static float next();
};

} // namespace fc