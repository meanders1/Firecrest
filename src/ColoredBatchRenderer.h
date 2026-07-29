#pragma once

#include "Color.h"
#include "Window.h"
#include "fiv.hpp"
#include "gl/IndexBuffer.h"
#include "gl/VertexArray.h"
#include "gl/VertexBufferLayout.h"
#include "glm/glm.hpp"
#include "res/ResourceManager.h"
#include <array>
#include <vector>

namespace fc {

class ColoredBatchRenderer {
private:
    struct Vertex {
        glm::vec2 position;
        GLuint RGBA; // A color packed into a uint (32 bit) in the format:
                     // RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
    };

private:
    const uint32_t INDICES_PER_QUAD = 6;
    const uint32_t VERTICES_PER_QUAD = 4;

private:
    std::vector<ColoredBatchRenderer::Vertex> _vertices;

    std::vector<GLuint> _indices;
    gl::IndexBuffer _IBO;
    gl::VertexBuffer _VBO;
    gl::VertexArray _VAO;
    res::ShaderHandle _shader;

    res::ResourceManager& _resourceManager;
    Window& _window;

private:
    void createIndicesForQuads(size_t quadCount);
    std::array<ColoredBatchRenderer::Vertex, 4>
    createQuad(const glm::vec2 position, const glm::vec2 scale, const fc::Color color);

public:
    ColoredBatchRenderer(Window& window, res::ResourceManager& resourceManager);
    void clearElements();
    void reserve(const size_t quadCount);
    void addQuad(const glm::vec2 position, const glm::vec2 scale, const fc::Color color);
    void draw();
};
} // namespace fc