#include "RenderTarget.h"

fc::gl::RenderTarget::RenderTarget(int width, int height) : _width(width), _height(height) {
    create();
}

fc::gl::RenderTarget::~RenderTarget() {
    destroy();
}

void fc::gl::RenderTarget::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);
    glViewport(0, 0, _width, _height);
}

void fc::gl::RenderTarget::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fc::gl::RenderTarget::bindTexture(size_t slot) const {
    _colorTexture.bind(slot);
}

void fc::gl::RenderTarget::clear(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void fc::gl::RenderTarget::create() {
    glGenFramebuffers(1, &_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

    // Color texture
    _colorTexture.setData(GL_RGBA8, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    _colorTexture.bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           _colorTexture.getHandle(), 0);

    // Depth buffer
    glGenRenderbuffers(1, &_depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              _depthRBO);

    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "fc::gl::RenderTarget framebuffer incomplete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fc::gl::RenderTarget::destroy() {
    if (_depthRBO)
        glDeleteRenderbuffers(1, &_depthRBO);
    if (_FBO)
        glDeleteFramebuffers(1, &_FBO);

    _depthRBO = 0;
    _FBO = 0;
}

void fc::gl::RenderTarget::resize(int width, int height) {
    if (width == _width && height == _height)
        return;

    _width = width;
    _height = height;

    destroy();
    create();
}
