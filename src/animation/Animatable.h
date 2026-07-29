#pragma once
#include "Lerp.h"
#include "TransitionFunction.h"
#include "core/Time.h"

namespace fc::animation {

template <typename T>
class Animatable {
public:
    TransitionFunction<T> transitionFunction;
    time::Duration duration;

private:
    T _lastValue, _targetValue;
    /// @brief The time a new value was assigned to the Animatable.
    time::Moment _setTime;

public:
    Animatable(T value, time::Duration animationDuration, TransitionFunction<T> transitionFunction)
        : _lastValue(value),
          _targetValue(value),
          transitionFunction(transitionFunction),
          duration(animationDuration),
          _setTime(time::now())
    {
    }

    Animatable(T value, time::Duration animationDuration)
        : _lastValue(value),
          _targetValue(value),
          transitionFunction(lerp<T>),
          duration(animationDuration),
          _setTime(time::now())
    {
    }

    Animatable(T value) : Animatable(value, time::Duration::fromSeconds(0.1)) {}

    Animatable& operator=(T newValue)
    {
        if (time::now() - _setTime >= duration) {
            _setTime = time::now();
            _lastValue = _targetValue;
            _targetValue = newValue;
        }
        else {
            auto current = get();
            _setTime = time::now();
            _lastValue = current;
            _targetValue = newValue;
        }

        return *this;
    }

    T get() const
    {
        float progress = 1.0f;
        if (duration.millis() > 0) {
            const auto deltaTime = time::now() - _setTime;
            progress = deltaTime.seconds() / duration.seconds();
            progress = std::clamp(progress, 0.0f, 1.0f);
        }
        return transitionFunction(_lastValue, _targetValue, progress);
    }

    operator T() const { return get(); }

    T operator*() const { return get(); }
};

} // namespace fc::animation