#pragma once
#include "VertexBufferLayout.h"
#include "glm/glm.hpp"

namespace fc::gl {
struct Vertex3D {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 tangent;

    bool operator==(const Vertex3D& other) const
    {
        return position == other.position && texCoord == other.texCoord && normal == other.normal
               && tangent == other.tangent;
    }

    bool operator!=(const Vertex3D& other) const
    {
        return position != other.position || texCoord != other.texCoord || normal != other.normal
               || tangent != other.tangent;
    }

    static VertexBufferLayout layout()
    {
        VertexBufferLayout layout;
        layout.push(GL_FLOAT, 3);
        layout.push(GL_FLOAT, 2);
        layout.push(GL_FLOAT, 3);
        layout.push(GL_FLOAT, 3);
        return layout;
    }
};
} // namespace fc::gl