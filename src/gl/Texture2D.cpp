#include "Texture2D.h"
#include "stb_image.h"
#include <iostream>

fc::gl::Texture2D::Texture2D() : _width(0), _height(0) {}

fc::gl::Texture2D::Texture2D(const std::string& path, bool blurred) : _width(0), _height(0) {
    stbi_set_flip_vertically_on_load(1);
    int bpp = 0;
    stbi_uc* imageData = stbi_load(path.c_str(), &_width, &_height, &bpp, 4);

    if (imageData == NULL) {
        std::cout << "Error loading texture: " << stbi_failure_reason() << ". Path: " << path
                  << std::endl;
    }

    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);
    if (blurred) {
        _minMagFilter = GL_LINEAR;
    } else {
        // Sharp
        _minMagFilter = GL_NEAREST;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _minMagFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapMode);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (imageData)
        stbi_image_free(imageData);
}

fc::gl::Texture2D::Texture2D(GLint textureFormat, GLsizei width, GLsizei height, GLenum dataFormat,
                             GLenum dataType, const void* data)
    : _width(width), _height(height) {
    glGenTextures(1, &_handle);
    setData(textureFormat, width, height, dataFormat, dataType, data);
}

void fc::gl::Texture2D::setData(GLint textureFormat, GLsizei width, GLsizei height,
                                GLenum dataFormat, GLenum dataType, const void* data) {
    _width = width;
    _height = height;

    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0, dataFormat, dataType, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _minMagFilter);

    unbind();
}

glm::uvec3 fc::gl::Texture2D::size() const {
    return {_width, _height, 1};
}
