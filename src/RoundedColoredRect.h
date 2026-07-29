#pragma once
#include "Color.h"
#include "Element.h"
#include "ShapeRenderer2D.h"

namespace fc {

class RoundedColoredRect : public Element {
public:
    Color color;
    float radius;
    ShapeRenderer2D& renderer;
    uint32_t quality;

public:
    RoundedColoredRect(alignment::ElementAlignment alignment, Color color, float radius,
                       ShapeRenderer2D& renderer)
        : Element(alignment), color(color), radius(radius), renderer(renderer), quality(32)
    {
    }

    void render(const Window& window, time::Duration delta) override
    {
        renderer.roundedRect(window, getPixelPosition(), getPixelSize(), color, radius, quality);
    }
};
} // namespace fc