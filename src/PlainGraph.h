#pragma once

#include <algorithm>
#include <vector>

#include "Color.h"
#include "Element.h"
#include "ShapeRenderer2D.h"
#include "core/Maths.h"

namespace fc {
class PlainGraph : public Element {
private:
    ShapeRenderer2D& _renderer;

    float _yMin = 0;
    float _yMax = 0;
    bool _fixedBounds = false;

public:
    // The data vector should have x-values in ascending order
    std::vector<glm::vec2> data;
    bool drawGrid = true;
    float lineWidth = 2.0f;

    PlainGraph(alignment::ElementAlignment alignment, ShapeRenderer2D& renderer)
        : Element(alignment), _renderer(renderer)
    {
    }

    // If min == max, the bounds are set to not be fixed.
    PlainGraph& setYBounds(float min, float max)
    {
        _fixedBounds = true;
        _yMin = min;
        _yMax = max;

        if (min == max) {
            _fixedBounds = false;
        }

        return *this;
    }

    virtual void render(const Window& window, time::Duration delta) override
    {
        if (data.size() == 0)
            return;

        const float dataMinX = data[0].x;
        const float dataMaxX = data.back().x;

        float dataMinY = _yMin;
        float dataMaxY = _yMax;
        if (!_fixedBounds) {
            dataMinY = std::min_element(data.begin(), data.end(), [](auto a, auto b) {
                           return a.y < b.y;
                       })->y;
            dataMaxY = std::max_element(data.begin(), data.end(), [](auto a, auto b) {
                           return a.y < b.y;
                       })->y;
        }
        auto minPos = getPixelPosition();
        auto maxPos = minPos + getPixelSize();

        // Draw grid
        const float spanX = dataMaxX - dataMinX;
        const float spanY = dataMaxY - dataMinY;

        const float linesIntervalX = powf(10, floorf(log10(spanX)));
        const float linesIntervalY = powf(10, floorf(log10(spanY)));

        std::vector<float> linesX;

        float startX = dataMinX + linesIntervalX;
        startX -= fmod(startX, linesIntervalX);

        for (float x = startX; x < dataMaxX; x += linesIntervalX) {
            linesX.push_back(x);
        }

        float startY = dataMinY + linesIntervalY;
        startY -= fmod(startY, linesIntervalY);

        std::vector<float> linesY;

        for (float y = startY; y < dataMaxY; y += linesIntervalY) {
            linesY.push_back(y);
        }
        const Color color = {0.2, 0.2, 0.2, 1};
        for (float datax : linesX) {
            const float x = maths::map(datax, dataMinX, dataMaxX, minPos.x, maxPos.x);
            _renderer.lineSegment(window, {x, minPos.y}, {x, maxPos.y}, color);
        }

        for (float datay : linesY) {
            const float y = maths::map(datay, dataMinY, dataMaxY, minPos.y, maxPos.y);
            _renderer.lineSegment(window, {minPos.x, y}, {maxPos.x, y}, color);
        }

        // Draw graph
        std::vector<glm::vec2> points;
        points.reserve(data.size());

        for (auto& d : data) {
            points.push_back({maths::map(d.x, dataMinX, dataMaxX, minPos.x, maxPos.x),
                              maths::map(d.y, dataMinY, dataMaxY, minPos.y, maxPos.y)});
        }
        _renderer.lineStrip(window, points, glm::vec4(0.1, 0.8, 0.3, 1), lineWidth);
    }
};
} // namespace fc