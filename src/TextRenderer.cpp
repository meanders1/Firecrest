#include "TextRenderer.h"
#include "gl/VertexBufferLayout.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fc {

TextRenderer::TextRenderer(const std::string& fontPath) : _charset(fontPath)
{
    // Compile the MSDF shader
    const char* VERTEX_SOURCE = R"(
        #version 330 core
        layout(location = 0) in vec3 pos;
        layout(location = 1) in vec2 uv;

        out vec2 TexCoords;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(pos, 1.0);
            TexCoords = uv;
        }
    )";

    const char* FRAGMENT_SOURCE = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D atlas;
        uniform vec4 textColor;

        float median(float r, float g, float b) {
            return max(min(r, g), min(max(r, g), b));
        }

        void main() {
            vec3 msd = texture(atlas, TexCoords).rgb;
            float sd = median(msd.r, msd.g, msd.b);
            float pixelRange = 2.0; // This should be 2.0 (matching the packer.setPixelRange)

            // Dynamic screen pixel range calculation
            // unitRange converts the MSDF distance to atlas texels
            vec2 unitRange = vec2(pixelRange) / vec2(textureSize(atlas, 0));
            
            // screenTexSize tells us how many atlas texels are in one screen pixel
            vec2 screenTexSize = vec2(1.0) / fwidth(TexCoords);
            
            // Multiply them to get the range in screen pixels
            float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);

            float opacity = clamp((sd - 0.5) * screenPxRange + 0.5, 0.0, 1.0);
            
            color = vec4(textColor.rgb, textColor.a * opacity);
        }
    )";

    _textShader.addStageSource(GL_VERTEX_SHADER, VERTEX_SOURCE);
    _textShader.addStageSource(GL_FRAGMENT_SHADER, FRAGMENT_SOURCE);
    _textShader.link();

    // Setup VAO/VBO
    _VAO.bind();
    gl::VertexBufferLayout layout;
    layout.push(GL_FLOAT, 3); // pos
    layout.push(GL_FLOAT, 2); // uv
    _VAO.addBuffer(_VBO, layout);
    _VBO.bind();
    // Reserve enough space for one string (can grow dynamically if needed)
    _VBO.setData(nullptr, sizeof(Vertex) * 1024 * 6, GL_DYNAMIC_DRAW);
    _VAO.unbind();
    _VBO.unbind();
}

void TextRenderer::renderText(const Window& window, const std::string& text, glm::vec3 pos,
                              float scale, Color color)
{
    renderText(static_cast<glm::vec2>(window.dimensions()), text, pos, scale, color);
}

void TextRenderer::renderText(glm::vec2 viewportSize, const std::string& text, glm::vec3 pos,
                              float scale, Color color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::ortho(0.0f, viewportSize.x, 0.0f, viewportSize.y);

    _textShader.bind();
    _textShader.setUniformMat4f("projection", projection);
    _textShader.setUniform4f("textColor", color.r, color.g, color.b, color.a);
    _textShader.setUniform1i("atlas", 0);
    _charset.atlas().bind(0);

    _VAO.bind();

    std::vector<Vertex> vertices;
    vertices.reserve(text.size() * 6);

    float x = pos.x;
    float y = pos.y;

    for (char c : text) {
        if (!std::isprint(c))
            continue;

        const auto& glyph = _charset.glyph(c);

        float xpos = x + glyph.bearing.x * scale;
        float ypos = y - (glyph.size.y - glyph.bearing.y) * scale;
        float w = glyph.size.x * scale;
        float h = glyph.size.y * scale;

        // Inside TextRenderer.cpp
        float u0 = glyph.uvMin.x; // Left
        float v0 = glyph.uvMin.y; // Bottom
        float u1 = glyph.uvMax.x; // Right
        float v1 = glyph.uvMax.y; // Top

        // Triangle 1
        vertices.push_back({{xpos, ypos + h, pos.z}, {u0, v1}}); // Top Left
        vertices.push_back({{xpos, ypos, pos.z}, {u0, v0}});     // Bottom Left
        vertices.push_back({{xpos + w, ypos, pos.z}, {u1, v0}}); // Bottom Right

        // Triangle 2
        vertices.push_back({{xpos, ypos + h, pos.z}, {u0, v1}});     // Top Left
        vertices.push_back({{xpos + w, ypos, pos.z}, {u1, v0}});     // Bottom Right
        vertices.push_back({{xpos + w, ypos + h, pos.z}, {u1, v1}}); // Top Right

        x += glyph.advance * scale;
    }

    // Upload all vertices at once
    _VBO.bind();
    _VBO.editData(vertices.data(), 0, vertices.size() * sizeof(Vertex));
    _VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

    _VAO.unbind();
    _VBO.unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
}

float TextRenderer::width(const std::string& text, float scale)
{
    float x = 0;
    for (char c : text) {
        if (!std::isprint(c))
            continue;
        x += _charset.glyph(c).advance * scale;
    }
    return x;
}

float TextRenderer::height(const std::string& text, float scale)
{
    float minY = 1e6f;
    float maxY = -1e6f;
    for (char c : text) {
        if (!std::isprint(c))
            continue;
        const auto& g = _charset.glyph(c);
        float yMin = (g.bearing.y - g.size.y) * scale;
        float yMax = g.bearing.y * scale;
        if (yMin < minY)
            minY = yMin;
        if (yMax > maxY)
            maxY = yMax;
    }
    return maxY - minY;
}

float TextRenderer::lineHeight(float scale)
{
    return _charset.lineHeight() * scale;
}

float TextRenderer::descenderHeight(float scale)
{
    return _charset.descender() * scale;
}

float TextRenderer::ascenderHeight(float scale)
{
    return _charset.ascender() * scale;
}

} // namespace fc