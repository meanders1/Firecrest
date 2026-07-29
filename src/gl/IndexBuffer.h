#pragma once
#include "Buffer.h"
#include <stdint.h>

namespace fc::gl {

class IndexBuffer : public Buffer<GL_ELEMENT_ARRAY_BUFFER> {
private:
    GLsizei _count = 0;

public:
    void setIndices(const GLuint* data, const GLsizei count, const GLenum usage = GL_STATIC_DRAW);

    inline GLsizei getCount() const { return _count; };
};
} // namespace fc::gl