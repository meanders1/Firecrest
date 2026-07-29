#pragma once
#include "Container.h"

namespace fc {
class VerticalCenterer : public Container {
public:
    VerticalCenterer() : Container(alignment::ElementAlignment())
    {
        this->alignment.width = [this](float parent1, float parent2) -> float {
            float minX = INFINITY;
            float maxX = 0;
            for (auto& child : children()) {
                const float childX = child->alignment.x(parent1, parent2);
                const float childWidth = child->alignment.width(parent1, parent2);
                if (childX < minX) {
                    minX = childX;
                }
                if (childX + childWidth > maxX) {
                    maxX = childX + childWidth;
                }
            }
            return maxX - minX;
        };
    }

    virtual void childCreated(fiv::ID id) override
    {
        auto& child = children()[id];
        child->alignment.y = [id, this](float parent1, float parent2) -> float {
            const float height = children()[id]->getPixelSize().y;
            return (parent1 - height) * 0.5f;
        };
    }

    virtual glm::vec2
    calculateChildPixelSize(const alignment::ElementAlignment& childAlignment) const override
    {
        return childAlignment.getPixelSize(parent().getPixelSize());
    }
};
} // namespace fc
