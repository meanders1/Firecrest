#include "ShapeRenderer2D.h"
#include "core/Maths.h"
#include "generators/RoundedRectGenerator.h"
#include "gl/VertexBufferLayout.h"
#include "glm/gtc/matrix_transform.hpp"

static constexpr const char* VERTEX_SHADER_SOURCE = R"(
#version 450 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in uint a_RGBA;

uniform mat4 u_ViewProj;

out vec4 v_Color;

void main() {
	uint r = (a_RGBA >> 24) & 255;
	uint g = (a_RGBA >> 16) & 255;
	uint b = (a_RGBA >> 8)  & 255;
	uint a =  a_RGBA        & 255;
	v_Color = vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, float(a) / 255.0);
	gl_Position = u_ViewProj * vec4(a_Position, 0.0, 1.0);
}
)";

static constexpr const char* FRAGMENT_SHADER_SOURCE = R"(
#version 450 core

layout (location=0) out vec4 o_Color;

in vec4 v_Color;

void main() {
	o_Color = v_Color;
}
)";

namespace fc {

ShapeRenderer2D::ShapeRenderer2D()
{
    m_Shader.addStageSource(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
    m_Shader.addStageSource(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
    m_Shader.link();
    m_Shader.bind();

    m_VAO.bind();

    gl::VertexBufferLayout layout;
    layout.push(GL_FLOAT, 2);        // Position
    layout.push(GL_UNSIGNED_INT, 1); // Color

    m_VAO.addBuffer(m_VBO, layout);
    m_VAO.addBuffer(m_IBO);

    m_VAO.unbind();
    m_VBO.unbind();
    m_IBO.unbind();
}

void ShapeRenderer2D::beforeRender(const Window& window) {}

void ShapeRenderer2D::afterRender(const Window& window) {}

void ShapeRenderer2D::renderFan(const Window& window, const std::vector<Vertex>& vertices)
{
    if (vertices.size() < 3)
        return;

    std::vector<GLuint> indices;
    const GLuint numTriangles = static_cast<GLuint>(vertices.size()) - 2;
    indices.reserve(numTriangles * 3);
    for (GLuint triangle = 1; triangle < numTriangles + 1; triangle++) {
        indices.push_back(0);
        indices.push_back(triangle);
        indices.push_back(triangle + 1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDisable(GL_DEPTH_TEST);

    m_IBO.setIndices(indices.data(), static_cast<GLsizei>(indices.size()));

    const int width = window.width();
    const int height = window.height();
    auto projection = window.orthographicProjection();

    m_VBO.setData(vertices.data(), vertices.size() * sizeof(ShapeRenderer2D::Vertex),
                  GL_STREAM_DRAW);

    m_Shader.bind();

    m_Shader.setUniformMat4f("u_ViewProj", projection);
    m_VAO.bind();
    m_IBO.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void ShapeRenderer2D::rect(const Window& window, glm::vec2 position, glm::vec2 scale, Color color)
{
    if (scale.x <= 0 || scale.y <= 0)
        return;

    const GLuint packedColor = color.toHex(true);

    ShapeRenderer2D::Vertex v1;
    v1.position = glm::vec2(0, 0) * scale + position;
    v1.RGBA = packedColor;

    ShapeRenderer2D::Vertex v2;
    v2.position = glm::vec2(1, 0) * scale + position;
    v2.RGBA = packedColor;

    ShapeRenderer2D::Vertex v3;
    v3.position = glm::vec2(1, 1) * scale + position;
    v3.RGBA = packedColor;

    ShapeRenderer2D::Vertex v4;
    v4.position = glm::vec2(0, 1) * scale + position;
    v4.RGBA = packedColor;

    renderFan(window, {v1, v2, v3, v4});
}

void fc::ShapeRenderer2D::roundedRect(const Window& window, glm::vec2 position, glm::vec2 scale,
                                      Color color, float radius, uint32_t quality)
{
    if (scale.x <= 0 || scale.y <= 0)
        return;

    if (radius * 2.0f > scale.x) {
        radius = scale.x / 2.0f;
    }
    if (radius * 2.0f > scale.y) {
        radius = scale.y / 2.0f;
    }

    std::vector<ShapeRenderer2D::Vertex> vertices;
    const GLuint packedColor = color.toHex(true);

    RoundedRectGenerator generator(scale, radius, quality);
    for (uint32_t i = 0; i < quality; i++) {
        vertices.push_back({generator.getPoint(i) + position, packedColor});
    }

    renderFan(window, vertices);
}

void ShapeRenderer2D::circle(const Window& window, glm::vec2 center, float radius, Color color,
                             uint32_t quality)
{
    if (radius < 0)
        radius = -radius;

    std::vector<ShapeRenderer2D::Vertex> vertices;
    const GLuint packedColor = color.toHex(true);

    CircleGenerator generator(radius, quality);
    for (uint32_t i = 0; i < quality; i++) {
        vertices.push_back({generator.getPoint(i) + center, packedColor});
    }

    renderFan(window, vertices);
}

void ShapeRenderer2D::lineStrip(const Window& window, std::vector<glm::vec2> points, Color color,
                                float thickness)
{
    if (points.size() < 2 || thickness <= 0.0f)
        return;

    const GLuint packedColor = color.toHex(true);
    std::vector<ShapeRenderer2D::Vertex> vertices;
    std::vector<GLuint> indices;

    auto getNormal = [](const glm::vec2& a, const glm::vec2& b) {
        glm::vec2 dir = glm::normalize(b - a);
        return glm::vec2(-dir.y, dir.x);
    };

    // Generate vertices with mitered joins
    for (size_t i = 0; i < points.size(); ++i) {
        glm::vec2 normalPrev, normalNext, miter;
        float miterLen = 1.0f;

        if (i == 0) {
            // Start cap
            normalNext = getNormal(points[i], points[i + 1]);
            miter = normalNext;
        }
        else if (i == points.size() - 1) {
            // End cap
            normalPrev = getNormal(points[i - 1], points[i]);
            miter = normalPrev;
        }
        else {
            normalPrev = getNormal(points[i - 1], points[i]);
            normalNext = getNormal(points[i], points[i + 1]);
            glm::vec2 tangent = glm::normalize(glm::normalize(points[i + 1] - points[i])
                                               + glm::normalize(points[i] - points[i - 1]));
            miter = glm::vec2(-tangent.y, tangent.x);

            // Compute miter length to avoid spikes
            float dot = glm::dot(miter, normalNext);
            if (fabs(dot) > 0.01f)
                miterLen = 1.0f / dot;
            else
                miterLen = 1.0f; // fallback for nearly parallel
        }

        glm::vec2 offset = miter * (thickness * 0.5f * miterLen);
        vertices.push_back({points[i] + offset, packedColor});
        vertices.push_back({points[i] - offset, packedColor});
    }

    // Indices for triangle strip
    for (GLuint i = 0; i < vertices.size(); ++i) {
        indices.push_back(i);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_IBO.setIndices(indices.data(), static_cast<GLsizei>(indices.size()));

    auto projection = window.orthographicProjection();
    m_VBO.setData(vertices.data(), vertices.size() * sizeof(ShapeRenderer2D::Vertex),
                  GL_STREAM_DRAW);

    m_Shader.bind();
    m_Shader.setUniformMat4f("u_ViewProj", projection);

    m_VAO.bind();
    m_IBO.bind();
    glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT,
                   nullptr);
}
void ShapeRenderer2D::lineSegment(const Window& window, glm::vec2 point1, glm::vec2 point2,
                                  Color color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    const GLuint packedColor = color.toHex(true);
    ShapeRenderer2D::Vertex v1{point1, packedColor};
    ShapeRenderer2D::Vertex v2{point2, packedColor};
    std::vector<ShapeRenderer2D::Vertex> vertices = {v1, v2};
    std::vector<GLuint> indices = {0, 1};

    m_IBO.setIndices(indices.data(), static_cast<GLsizei>(indices.size()));

    auto projection = window.orthographicProjection();
    m_VBO.setData(vertices.data(), vertices.size() * sizeof(ShapeRenderer2D::Vertex),
                  GL_STREAM_DRAW);

    m_Shader.bind();
    m_Shader.setUniformMat4f("u_ViewProj", projection);

    m_VAO.bind();
    m_IBO.bind();
    glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}
} // namespace fc
