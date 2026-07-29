#pragma once

#include "Element.h"
#include "gl/Shader.h"
#include "gl/Shape.h"
#include "glm/gtc/matrix_transform.hpp"

namespace fc {
class ShaderQuad : public Element {
private:
    struct Vertex {
        glm::vec2 position;

        Vertex(float x, float y) : position(x, y) {}

        static gl::VertexBufferLayout layout()
        {
            gl::VertexBufferLayout l;
            l.push(GL_FLOAT, 2);
            return l;
        }
    };

public:
    gl::Shader shader;

    std::function<void(gl::Shader&)> beforeRender;

private:
    gl::Shape<Vertex> _shape;

public:
    /// @param alignment The alignment of the quad.
    /// @param shaderCode The fragment shader code. The fragment shader have the
    /// following inputs available:
    ///
    /// - in vec2 uv; // The UV coordinates of the fragment, ranging from (0,0)
    /// at bottom-left to (1,1) at top-right.
    ///
    /// - in vec2 pixelPosition; // The pixel position of the fragment in screen
    /// space
    ShaderQuad(alignment::ElementAlignment alignment, const std::string& shaderCode,
               std::function<void(gl::Shader&)> beforeRender)
        : Element(alignment), beforeRender(beforeRender)
    {
        const std::string vertexShader = R"(
                #version 330 core
                layout(location = 0) in vec2 aPos;

                uniform mat4 transform;
                uniform mat4 projection;

                out vec2 uv;
                out vec2 pixelPosition;

                void main()
                {
                    uv = aPos.xy;
                    pixelPosition = (transform * vec4(aPos, 0.0, 1.0)).xy;
                    gl_Position = projection * transform * vec4(aPos, 0.0, 1.0);
                }
            )";

        shader.addStageSource(GL_VERTEX_SHADER, vertexShader);
        shader.addStageSource(GL_FRAGMENT_SHADER, shaderCode);
        shader.link();

        std::vector<Vertex> vertices = {
            {0.0f, 0.0f}, // bottom left
            {1.0f, 0.0f}, // bottom right
            {1.0f, 1.0f}, // top right
            {0.0f, 1.0f}  // top left
        };

        std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0};

        _shape.setVertices(vertices, indices, GL_STATIC_DRAW);
    }

    ShaderQuad(alignment::ElementAlignment alignment, const std::string& shaderCode)
        : ShaderQuad(alignment, shaderCode, nullptr)
    {
    }

    void render(const Window& window, time::Duration delta) override
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        shader.bind();

        if (beforeRender) {
            beforeRender(shader);
        }

        glm::mat4 projection = window.orthographicProjection();
        shader.setUniformMat4f("projection", projection);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(getPixelPosition(), 0.0f));
        transform = glm::scale(transform, glm::vec3(getPixelSize(), 1.0f));
        shader.setUniformMat4f("transform", transform);

        _shape.vao.bind();
        _shape.ibo.bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_shape.ibo.getCount()), GL_UNSIGNED_INT,
                       nullptr);
        _shape.vao.unbind();
        _shape.ibo.unbind();
    }
};
} // namespace fc