#pragma once
#include "Color.h"
#include "Window.h"
#include "gl/IndexBuffer.h"
#include "gl/VertexArray.h"
#include "gl/VertexBufferLayout.h"
#include "res/ResourceManager.h"
#include <array>
#include <string>
#include <vector>

namespace fc {

class TexturedBatchRenderer {
private:
    struct Vertex {
        glm::vec2 position;
        GLuint RGBA;
        GLuint texIDTexCoordIndex;
    };

private:
    const uint32_t INDICES_PER_QUAD = 6;
    const uint32_t VERTICES_PER_QUAD = 4;

private:
    std::vector<TexturedBatchRenderer::Vertex> _vertices;
    res::ResourceManager& _resourceManager;

    glm::mat4 _view;
    glm::mat4 _projection;

    std::vector<res::TextureHandle> _textures;
    std::vector<GLuint> _indices;
    gl::IndexBuffer _IBO;
    gl::VertexBuffer _VBO;
    gl::VertexArray _VAO;
    res::ShaderHandle _shader;

private:
    void createIndicesForQuads(size_t quadCount);
    std::array<TexturedBatchRenderer::Vertex, 4> createQuad(const glm::vec2 position,
                                                            const glm::vec2 scale,
                                                            const Color& color,
                                                            const uint32_t textureID);

public:
    TexturedBatchRenderer(Window& window, res::ResourceManager& resourceManager);
    void addTexture(const std::string textureFile, const bool blurred);
    void clearElements();
    void draw(const Window& window);
    void reserve(const size_t count);
    void addQuad(const glm::vec2 position, const glm::vec2 scale, const Color& color,
                 const uint32_t textureIndex);
};
} // namespace fc
