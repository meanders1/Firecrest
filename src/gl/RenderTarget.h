#pragma once
#include "OpenGL.h"
#include "Texture2D.h"
#include <iostream>

namespace fc::gl {

class RenderTarget {
private:
    GLuint _FBO = 0;
    GLuint _depthRBO = 0;
    Texture2D _colorTexture;

    int _width = 0;
    int _height = 0;

public:
    RenderTarget(int width, int height);
    ~RenderTarget();

    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    void bind() const;
    static void unbind();
    void bindTexture(size_t slot) const;

    void resize(int width, int height);
    void clear(float r, float g, float b, float a = 1.0f) const;

    Texture2D& texture() { return _colorTexture; }
    int width() const { return _width; }
    int height() const { return _height; }

private:
    void create();
    void destroy();
};

} // namespace fc::gl
