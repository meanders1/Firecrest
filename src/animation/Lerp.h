#pragma once

#include "TransitionFunction.h"

namespace fc::animation {

template <typename T>
T lerp(T start, T end, float progress)
{
    return start + (end - start) * progress;
}

} // namespace fc::animation