#pragma once
#include "Charset.h"
#include "Color.h"
#include "Renderer.h"
#include "Window.h"
#include "fiv.hpp"
#include "gl/Shader.h"
#include "gl/Texture2D.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"
#include "glm/glm.hpp"
#include <map>
#include <memory>
#include <string>

namespace fc {
class TextRenderer : public Renderer {
private:
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 texCoord;
    };

private:
    gl::Shader _textShader;
    gl::VertexArray _VAO;
    gl::VertexBuffer _VBO;

    Charset _charset;

public:
    TextRenderer(const std::string& fontPath);

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    void renderText(const Window& window, const std::string& text, glm::vec3 pos, float scale,
                    Color color);
    void renderText(glm::vec2 viewportSize, const std::string& text, glm::vec3 pos, float scale,
                    Color color);
    float width(const std::string& text, float scale);
    float height(const std::string& text, float scale);
    float lineHeight(float scale);
    float descenderHeight(float scale);
    float ascenderHeight(float scale);

    virtual void beforeRender(const fc::Window& window) {}
    virtual void afterRender(const fc::Window& window) {}
};
} // namespace fc