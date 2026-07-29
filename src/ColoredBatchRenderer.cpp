#include "ColoredBatchRenderer.h"
#include "Color.h"
#include "glm/gtc/matrix_transform.hpp"
#include "res/ResourceManager.h"

static constexpr const char* VERTEX_SOURCE = R"(
#version 450 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in uint a_RGBA;

uniform mat4 u_ViewProj;
uniform mat4 u_Transform;

out vec4 v_Color;

void main() {
	uint r =  a_RGBA        & 255;
	uint g = (a_RGBA >> 8)  & 255;
	uint b = (a_RGBA >> 16) & 255;
	uint a = (a_RGBA >> 24) & 255;
	v_Color = vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, float(a) / 255.0);
	gl_Position = u_ViewProj * u_Transform * vec4(a_Position, 0.0, 1.0);
}
)";

static constexpr const char* FRAGMENT_SOURCE = R"(
#version 450 core

layout (location=0) out vec4 o_Color;

in vec4 v_Color;

void main() {
	o_Color = v_Color;
}
)";

namespace fc {
void ColoredBatchRenderer::createIndicesForQuads(size_t quadCount)
{
    if (quadCount * INDICES_PER_QUAD == _indices.size())
        return;
    _indices.resize(quadCount * INDICES_PER_QUAD);

    uint32_t offset = 0;
    for (size_t i = 0; i < quadCount * INDICES_PER_QUAD; i += 6) {
        _indices[i + 0] = 0 + offset;
        _indices[i + 1] = 1 + offset;
        _indices[i + 2] = 2 + offset;

        _indices[i + 3] = 2 + offset;
        _indices[i + 4] = 3 + offset;
        _indices[i + 5] = 0 + offset;

        offset += 4;
    }

    _IBO.setIndices(_indices.data(), static_cast<GLsizei>(_indices.size()));
}

std::array<ColoredBatchRenderer::Vertex, 4>
ColoredBatchRenderer::createQuad(const glm::vec2 position, const glm::vec2 scale,
                                 const fc::Color color)
{
    GLuint packedColor = color.toHex(true);

    ColoredBatchRenderer::Vertex v1;
    v1.position = glm::vec2(-0.5f, -0.5f) * scale + position;
    v1.RGBA = packedColor;

    ColoredBatchRenderer::Vertex v2;
    v2.position = glm::vec2(0.5f, -0.5f) * scale + position;
    v2.RGBA = packedColor;

    ColoredBatchRenderer::Vertex v3;
    v3.position = glm::vec2(0.5f, 0.5f) * scale + position;
    v3.RGBA = packedColor;

    ColoredBatchRenderer::Vertex v4;
    v4.position = glm::vec2(-0.5f, 0.5f) * scale + position;
    v4.RGBA = packedColor;

    return {v1, v2, v3, v4};
}

ColoredBatchRenderer::ColoredBatchRenderer(Window& window, res::ResourceManager& resourceManager)
    : _window(window), _resourceManager(resourceManager)
{
    _shader = resourceManager.loadShaderSource(VERTEX_SOURCE, FRAGMENT_SOURCE);
    _shader->bind();

    _VAO.bind();
    _VBO.bind();

    gl::VertexBufferLayout layout;
    layout.push(GL_FLOAT, 2); // Position
    // layout.push(GL_FLOAT, 4, GL_FALSE); // Color
    layout.push(GL_UNSIGNED_INT, 1); // Color

    _VAO.addBuffer(_VBO, layout);
    _VAO.addBuffer(_IBO);

    _VAO.unbind();
    _VBO.unbind();
    _IBO.unbind();
}

void ColoredBatchRenderer::clearElements()
{
    _vertices.clear();
}

void ColoredBatchRenderer::draw()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    _VBO.setData(_vertices.data(), _vertices.size() * sizeof(ColoredBatchRenderer::Vertex),
                 GL_STREAM_DRAW);

    _shader->bind();

    const int width = _window.width();
    const int height = _window.height();
    const glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    const glm::mat4 projection
        = glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -1.0f, 1.0f);

    _shader->setUniformMat4f("u_ViewProj", projection);
    _shader->setUniformMat4f("u_Transform", view);
    _VAO.bind();
    _IBO.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, nullptr);
}

void ColoredBatchRenderer::reserve(const size_t quadCount)
{
    _vertices.reserve(quadCount * VERTICES_PER_QUAD);
    createIndicesForQuads(quadCount);

    _VBO.setData(nullptr, quadCount * sizeof(ColoredBatchRenderer::Vertex) * VERTICES_PER_QUAD,
                 GL_STREAM_DRAW);
}

void ColoredBatchRenderer::addQuad(const glm::vec2 position, const glm::vec2 scale,
                                   const fc::Color color)
{
    std::array<ColoredBatchRenderer::Vertex, 4> quad = createQuad(position, scale, color);
    _vertices.push_back(quad[0]);
    _vertices.push_back(quad[1]);
    _vertices.push_back(quad[2]);
    _vertices.push_back(quad[3]);
}
} // namespace fc