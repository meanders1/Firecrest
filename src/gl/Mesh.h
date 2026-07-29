#pragma once

#include "Shape.h"
#include "Vertex3D.h"
#include <vector>

namespace fc::gl {

class Mesh : public Shape<Vertex3D> {
public:
    Mesh(const std::vector<Vertex3D>& vertices, const std::vector<GLuint>& indices)
        : Shape(vertices, indices, GL_STATIC_DRAW)
    {
    }

    Mesh& operator=(Mesh&& other) noexcept = default;
    Mesh(Mesh&& other) noexcept = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};

} // namespace fc::gl