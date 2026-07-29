#pragma once

#include "Texture.h"

namespace fc::gl {

class Texture2D : public Texture<GL_TEXTURE_2D> {
private:
    int _width, _height;
    GLenum _wrapMode = GL_CLAMP_TO_EDGE;
    GLenum _minMagFilter = GL_LINEAR;

public:
    Texture2D();
    Texture2D(const std::string& path, bool blurred);
    Texture2D(GLint textureFormat, GLsizei width, GLsizei height, GLenum dataFormat,
              GLenum dataType, const void* data);

    void setData(GLint textureFormat, GLsizei width, GLsizei height, GLenum dataFormat,
                 GLenum dataType, const void* data);

    inline int width() const { return _width; }
    inline int height() const { return _height; }

    glm::uvec3 size() const override;
};
} // namespace fc::gl
