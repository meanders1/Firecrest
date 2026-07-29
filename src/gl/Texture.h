#pragma once
#include "OpenGL.h"
#include "glm/glm.hpp"
#include <string>

namespace fc::gl {

template <GLenum Dimension> class Texture {
protected:
    GLuint _handle;

public:
    Texture() { glGenTextures(1, &_handle); }

    ~Texture() {
        if (_handle != 0) {
            glDeleteTextures(1, &_handle);
        }
    }

    // move assignment
    Texture& operator=(Texture&& other) {
        std::swap(_handle, other._handle);
        return *this;
    }
    // move constructor
    Texture(Texture&& other) {
        _handle = other._handle;
        other._handle = 0;
    }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(size_t slot) const {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot));
        glBindTexture(Dimension, _handle);
    }
    inline void bind() const { glBindTexture(Dimension, _handle); }
    static void unbind() { glBindTexture(Dimension, 0); }

    inline GLuint getHandle() const { return _handle; }

protected:
    virtual glm::uvec3 size() const = 0;
};

} // namespace fc::gl
