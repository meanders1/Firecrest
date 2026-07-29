#pragma once
#include "Container.h"

namespace fc {
class HorisontalCenterer : public Container {
public:
    HorisontalCenterer() : Container(alignment::ElementAlignment())
    {
        this->alignment.height = [this](float parent1, float parent2) -> float {
            float minY = INFINITY;
            float maxY = 0;
            for (auto& child : children()) {
                const float childY = child->alignment.y(parent1, parent2);
                const float childHeight = child->alignment.height(parent1, parent2);
                if (childY < minY) {
                    minY = childY;
                }
                if (childY + childHeight > maxY) {
                    maxY = childY + childHeight;
                }
            }
            return maxY - minY;
        };
    }

    virtual void childCreated(fiv::ID id) override
    {
        auto& child = children()[id];
        child->alignment.x = [id, this](float parent1, float parent2) -> float {
            const float width = children()[id]->getPixelSize().x;
            return (parent1 - width) * 0.5f;
        };
    }

    virtual glm::vec2
    calculateChildPixelSize(const alignment::ElementAlignment& childAlignment) const override
    {
        return childAlignment.getPixelSize(parent().getPixelSize());
    }
};
} // namespace fc
