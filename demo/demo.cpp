#include "firecrest.h"
#include <algorithm>
#include <execution>

using namespace fc;

int main()
{
    WindowProperties properties;
    properties.width = 1400;
    properties.height = 800;
    properties.vsync = true;
    properties.resizable = true;
    properties.antialiasing = true;
    properties.title = "UI Example";

    Window window(properties);

    Display display(window);
    window.clearColor(glm::vec4(1, 0, 1, 1.0f));

    res::ResourceManager res;

    ShapeRenderer2D& shapeRenderer = display.createRenderer<ShapeRenderer2D>();
    TextRenderer& textRenderer
        = display.createRenderer<TextRenderer>(RESOURCES_PATH "JetBrainsMono-Regular.ttf");

    auto& grid = display.createChild<UniformGrid>(alignment::ElementAlignment(), 2, 2);

    auto& v1 = grid.at(0, 0);
    auto& v2 = grid.at(0, 1);
    auto& v3 = grid.at(1, 0);
    auto& v4 = grid.at(1, 1);

    auto& textInput = v1.createChild<TextInput>(
        alignment::ElementAlignment(), Color(1, 1, 1, 1), Color(0.1, 0.1, 0.1, 1), 24.0f,
        "This is a text input field. Try it!", shapeRenderer, textRenderer);

    auto& graph
        = v2.createChild<Graph>(alignment::ElementAlignment(), shapeRenderer, textRenderer, 16.0f);
    std::vector<glm::vec2> graphData;
    float graphTimeOffset = 0.0f;

    v3.createChild<ColoredRect>(alignment::ElementAlignment(), Color(0, 0, 0, 1));
    auto& v4rect
        = v4.createChild<ColoredRect>(alignment::ElementAlignment(), Color(0.05, 0.05, 0.05, 1));

    auto& gradient = v3.createChild<ShaderQuad>(alignment::ElementAlignment(),
                                                R"(
#version 330 core

out vec4 FragColor;

in vec2 uv;
in vec2 pixelPosition;

void main()
{
    vec3 color = vec3(uv, 1.0);

    float spacing = 20.0;
    float dotRadius = 1.0;

    vec2 hex = vec2(
        (2.0/3.0) * pixelPosition.x,
        (-1.0/3.0) * pixelPosition.x + (sqrt(3.0)/3.0) * pixelPosition.y
    ) / spacing;

    vec3 cube = vec3(hex, -hex.x - hex.y);
    vec3 rcube = round(cube);

    vec3 diff = abs(rcube - cube);

    if(diff.x > diff.y && diff.x > diff.z)
        rcube.x = -rcube.y - rcube.z;
    else if(diff.y > diff.z)
        rcube.y = -rcube.x - rcube.z;

    vec2 hexCenter = spacing * vec2(
        1.5 * rcube.x,
        (sqrt(3.0)/2.0) * rcube.x + sqrt(3.0) * rcube.y
    );

    vec2 snapped = floor(hexCenter + 0.5);

    float d = distance(pixelPosition, snapped);
    if(d < dotRadius)
        color = vec3(0.0);

    FragColor = vec4(color, 1.0);
}
)");

    auto& button
        = v4.createChild<VerticalCenterer>().createChild<HorisontalCenterer>().createChild<Button>(
            alignment::ElementAlignment()
                .setWidth(alignment::Pixels(200))
                .setHeight(alignment::Relative(0.5f)),
            Color(1, 0, 0, 1), Color(0.8, 0, 0, 1), Color(0.5, 0, 0, 1), Color(1, 1, 1, 1), 16.0f,
            "Press me!",
            [&]() {
                std::cout << "Button Pressed!" << std::endl;
                v4rect.color = Color(fc::Random::next(), fc::Random::next(), fc::Random::next());
            },
            shapeRenderer, textRenderer);

    time::Moment lastTime = time::now();
    while (!window.shouldClose()) {
        time::Moment now = time::now();
        time::Duration delta = now - lastTime;
        lastTime = now;

        graphData.push_back({graphTimeOffset, sin(graphTimeOffset) * graphTimeOffset});
        graph.setData(graphData);
        graphTimeOffset += 0.15f;

        window.clearScreen();
        display.render();
        window.display();

        // Include performance metrics in the title
        window.setTitle(
            "UI Example (" + std::to_string(delta.millis()) + "ms" + " fps: "
            + std::to_string((int)maths::round(1.0f / static_cast<float>(delta.seconds()), 1))
            + ")");
    }

    return 0;
}