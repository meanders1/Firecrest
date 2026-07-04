#pragma once

#include "Container.h"
#include "HorisontalCenterer.h"
#include "PlainGraph.h"
#include "Text.h"

namespace fc {
class Graph : public Container {
public:
    PlainGraph& graph;
    Text& minMaxText;
    ColoredRect& background;

public:
    Graph(alignment::ElementAlignment alignment, ShapeRenderer2D& shapeRenderer,
          TextRenderer& textRenderer, float textSize)
        : Container(alignment),
          background(
              createChild<ColoredRect>(alignment::ElementAlignment(), glm::vec4(0, 0, 0, 1))),
          graph(createChild<PlainGraph>(
              alignment::ElementAlignment()
                  .setHeight([textSize, this](float parent1, float parent2) {
                      return parent1 - minMaxText.alignment.height(parent1, parent2);
                  })
                  .setY([this](float parent1, float parent2) {
                      return minMaxText.alignment.height(parent1, parent2);
                  }),
              shapeRenderer)),
          minMaxText(createChild<HorisontalCenterer>().createChild<Text>(
              alignment::ElementAlignment(), glm::vec4(1, 1, 1, 1), textSize, "Min: N/A Max: N/A",
              textRenderer)) {
        minMaxText.wrapMode = Text::WrapMode::NoWrap;
        minMaxText.wrapTightly = true;
    }

    virtual void render(const Window& window, time::Duration delta) override {
        glDisable(GL_DEPTH_TEST);

        background.render(window, delta);
        graph.render(window, delta);
        minMaxText.render(window, delta);
    }

    void setData(const std::vector<glm::vec2>& data) {
        graph.data = data;

        if (data.size() == 0) {
            minMaxText.text = "Min: N/A Max: N/A";
            return;
        }

        const float dataMinY = std::min_element(data.begin(), data.end(), [](auto a, auto b) {
                                   return a.y < b.y;
                               })->y;
        const float dataMaxY = std::max_element(data.begin(), data.end(), [](auto a, auto b) {
                                   return a.y < b.y;
                               })->y;

        minMaxText.text = "Min: " + std::to_string(dataMinY) + " Max: " + std::to_string(dataMaxY);
    }
};
} // namespace fc