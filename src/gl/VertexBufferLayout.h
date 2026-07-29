#pragma once

#include "OpenGL.h"
#include <vector>

namespace fc::gl {

struct VertexBufferElement {
    GLenum type;
    GLint count;
    GLboolean normalized;
    GLuint offset;

    static GLint sizeOfType(GLenum type)
    {
        switch (type) {
        case GL_BOOL:
            return sizeof(GLboolean);
        case GL_BYTE:
            return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);
        case GL_SHORT:
            return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
            return sizeof(GLint);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_FIXED:
            return sizeof(GLfixed);
        case GL_HALF_FLOAT:
            return sizeof(GLhalf);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        }
        return -1;
    }

    static bool isInteger(GLenum type)
    {
        switch (type) {
        case GL_BOOL:
            return false;
        case GL_BYTE:
            return true;
        case GL_UNSIGNED_BYTE:
            return true;
        case GL_SHORT:
            return true;
        case GL_UNSIGNED_SHORT:
            return true;
        case GL_INT:
            return true;
        case GL_UNSIGNED_INT:
            return true;
        case GL_FIXED:
            return true;
        case GL_HALF_FLOAT:
            return false;
        case GL_FLOAT:
            return false;
        case GL_DOUBLE:
            return false;
        }
        // Should not reach this return
        return false;
    }
};

class VertexBufferLayout {
private:
    std::vector<VertexBufferElement> _elements;
    GLuint _stride = 0;

public:
    void push(GLenum type, GLint count, GLboolean normalized = GL_FALSE)
    {
        _elements.push_back({type, count, normalized, _stride});
        _stride += count * VertexBufferElement::sizeOfType(type);
    }

    inline const std::vector<VertexBufferElement>& elements() const { return _elements; }

    inline GLuint getStride() const { return _stride; }
};

} // namespace fc::gl
