#pragma once

#include "Window.h"

namespace fc {
class Renderer {
public:
    virtual ~Renderer() {}
    virtual void beforeRender(const fc::Window& window) = 0;
    virtual void afterRender(const fc::Window& window) = 0;
};
} // namespace fc