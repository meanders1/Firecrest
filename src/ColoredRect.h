#pragma once
#include "Color.h"
#include "Element.h"
#include "ShapeRenderer2D.h"
#include "animation/Animatable.h"
#include "gl/Painter.h"
#include "gl/Shape.h"
#include "glm/gtc/matrix_transform.hpp"

inline const char* VERTEX_SHADER_SOURCE = R"(
#version 330 core
layout(location = 0) in vec2 aPosition;

uniform mat4 uProjection;
uniform mat4 uTransform;

void main()
{
    gl_Position = uProjection * uTransform * vec4(aPosition, 0.0, 1.0);
}
)";

inline const char* FRAGMENT_SHADER_SOURCE = R"(
#version 330 core

out vec4 FragColor;

uniform vec4 uColor;

void main()
{
    FragColor = uColor;
}
)";

namespace fc {

class ColoredRect : public Element {
public:
    struct Vertex {
        glm::vec2 position;

        static gl::VertexBufferLayout layout()
        {
            gl::VertexBufferLayout layout;
            layout.push(GL_FLOAT, 2, GL_FALSE);
            return layout;
        }
    };

public:
    animation::Animatable<Color> color;

    gl::Shape<Vertex> shape;
    gl::Painter<Vertex> painter;

public:
    ColoredRect(alignment::ElementAlignment alignment, Color color)
        : ColoredRect(
              alignment, color,
              gl::Painter<Vertex>(
                  std::move(gl::Shader(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE)),
                  [this](const Window& window, const gl::Shape<Vertex>&, const gl::Shader& shader) {
                      const auto rect = getPixelRectangle();

                      const glm::mat4 ortho = window.orthographicProjection();
                      shader.setUniformMat4f("uProjection", ortho);
                      const Color col = this->color.get();
                      shader.setUniform4f("uColor", col.r, col.g, col.b, col.a);
                      shader.setUniformMat4f(
                          "uTransform",
                          glm::translate(glm::mat4(1.0f), glm::vec3(rect.position, 0.0f))
                              * glm::scale(glm::mat4(1.0f), glm::vec3(rect.size, 1.0f)));
                  }))
    {
    }

    ColoredRect(alignment::ElementAlignment alignment, Color color, gl::Painter<Vertex>&& painter)
        : Element(alignment), color(color), painter(std::move(painter))
    {
        const std::vector<Vertex> vertices = {{{0, 0}}, {{1, 0}}, {{1, 1}}, {{0, 1}}};
        const std::vector<GLuint> indices = {0, 1, 2, 3};

        shape.ibo.setIndices(indices.data(), indices.size());
        shape.vbo.setData(vertices.data(), sizeof(Vertex) * vertices.size(), GL_DYNAMIC_DRAW);
        shape.drawMode = GL_TRIANGLE_FAN;
    }

    void render(const Window& window, time::Duration delta) override
    {
        painter.draw(window, shape);
    }
};
} // namespace fc
