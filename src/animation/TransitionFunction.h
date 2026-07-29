#pragma once

#include <functional>

namespace fc::animation {

/// @brief A function that defines how to transition between two values over time.
/// @tparam T The type of the values being transitioned.
/// @param start The starting value.
/// @param end The ending value.
/// @param progress A value between 0.0 and 1.0 representing the progress of the transition.
/// @return The interpolated value between start and end based on the progress.
template <typename T>
using TransitionFunction = std::function<T(T, T, float)>;

} // namespace fc::animation