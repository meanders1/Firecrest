#pragma once

#include "VertexArray.h"
#include <vector>

namespace fc::gl {

template <typename T>
concept VertexType = requires {
    { T::layout() } -> std::same_as<VertexBufferLayout>;
};

/// @brief A Shape holds geometry, as well as information about how that geometry is structured
/// (draw mode and index buffer).
template <VertexType VertexT>
class Shape {
public:
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ibo;

    GLenum drawMode;

public:
    Shape& operator=(Shape&& other) noexcept;
    Shape(Shape&& other) noexcept;

    Shape(const Shape&) = delete;
    Shape& operator=(const Shape&) = delete;

    Shape();
    Shape(const std::vector<VertexT>& vertices, const std::vector<GLuint>& indices,
          GLenum usage = GL_STATIC_DRAW, GLenum drawMode = GL_TRIANGLES);

    void setVertices(const std::vector<VertexT>& vertices, const std::vector<GLuint>& indices,
                     GLenum usage = GL_STATIC_DRAW, GLenum drawMode = GL_TRIANGLES);
};

// === Implementation ===

template <VertexType VertexT>
Shape<VertexT>& Shape<VertexT>::operator=(Shape&& other) noexcept
{
    if (this != &other) {
        vao = std::move(other.vao);
        vbo = std::move(other.vbo);
        ibo = std::move(other.ibo);

        drawMode = other.drawMode;
    }

    return *this;
}

template <VertexType VertexT>
Shape<VertexT>::Shape(Shape&& other) noexcept
    : vao(std::move(other.vao)),
      vbo(std::move(other.vbo)),
      ibo(std::move(other.ibo)),
      drawMode(other.drawMode)
{
}

template <VertexType VertexT>
Shape<VertexT>::Shape() : drawMode(GL_TRIANGLES)
{
    vao.bind();

    VertexBufferLayout layout = VertexT::layout();
    vao.addBuffer(vbo, layout);
    vao.addBuffer(ibo);

    vao.unbind();
}

template <VertexType VertexT>
Shape<VertexT>::Shape(const std::vector<VertexT>& vertices, const std::vector<GLuint>& indices,
                      GLenum usage, GLenum drawMode)
    : Shape()
{
    setVertices(vertices, indices, usage, drawMode);
}

template <VertexType VertexT>
void Shape<VertexT>::setVertices(const std::vector<VertexT>& vertices,
                                 const std::vector<GLuint>& indices, GLenum usage,
                                 GLenum drawMode)
{
    this->drawMode = drawMode;
    vbo.setData(vertices.data(), sizeof(VertexT) * vertices.size(), usage);
    ibo.setIndices(indices.data(), indices.size());
}

} // namespace fc::gl