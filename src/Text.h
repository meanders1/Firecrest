#pragma once
#include "Color.h"
#include "Container.h"
#include "Element.h"
#include "TextRenderer.h"
#include "core/StringUtils.h"
#include "core/Tracked.h"
#include "gl/RenderRegion.h"
#include <cctype>

namespace fc {

class Text : public Element {
public:
    enum class WrapMode {
        NoWrap, // Text will not wrap, it will be cut off if too long
        Wrap,   // Text will wrap to the next line if too long
        // WordWrap    // Text will wrap at word boundaries if too long
    };

public:
    TextRenderer& renderer;
    alignment::AlignmentFunction defaultWidth;
    alignment::AlignmentFunction defaultHeight;

    Tracked<Color> color;
    Tracked<float> textSize;
    Tracked<WrapMode> wrapMode{WrapMode::Wrap};
    Tracked<bool> wrapTightly{false};
    std::string text;

private:
    // vector<pair<lineText, position relative to self>>
    std::vector<std::pair<std::string, glm::vec2>> _linesCache;

    glm::vec2 _lastParentSize = {-1, -1};
    std::string _lastText;

public:
    Text(alignment::ElementAlignment alignment, Color textColor, float textSize,
         const std::string& text, TextRenderer& textRenderer)
        : Element(alignment),
          textSize(textSize),
          renderer(textRenderer),
          color(textColor),
          defaultWidth(alignment.width),
          defaultHeight(alignment.height),
          text(text)
    {
    }

    virtual void render(const Window& window, time::Duration delta) override
    {
        bool shouldRebuild = false;

        const glm::vec2 parentSize = parent().getPixelSize();
        if (parentSize != _lastParentSize) {
            shouldRebuild = true;
            _lastParentSize = parentSize;
        }

        if (text != _lastText) {
            shouldRebuild = true;
            _lastText = text;
        }

        if (color.isModified() || textSize.isModified() || wrapMode.isModified()
            || wrapTightly.isModified()) {
            shouldRebuild = true;
        }

        if (shouldRebuild) {
            buildLinesCache();
        }

        if (text.empty())
            return;

        const Rectangle rect = getPixelRectangle();
        gl::RenderRegion::push(rect, gl::RenderRegion::Mode::Scissor);
        const glm::vec2 pos = rect.position;
        const glm::vec2 size = rect.size;
        const float top = pos.y + size.y;
        const float bottom = pos.y;

        for (auto& line : _linesCache) {
            glm::vec2 linePos = line.second + pos;
            float lineY = linePos.y;

            // Only render if line is within vertical bounds
            if (lineY <= top && lineY >= bottom) {
                renderer.renderText(window, line.first, glm::vec3(linePos, 0), textSize, color);
            }
        }

        gl::RenderRegion::pop();
    }

    void buildLinesCache()
    {
        _linesCache.clear();
        const std::string string = text;
        const float maxLineWidth
            = this->defaultWidth(parent().getPixelSize().x, parent().getPixelSize().y);
        const float lineHeight = renderer.lineHeight(textSize);

        bool isWrapped = false;

        if (wrapMode == WrapMode::NoWrap) {
            _linesCache.push_back({string, {0, -lineHeight}});
        }
        else if (wrapMode == WrapMode::Wrap) {
            float yPos = 0;

            auto segments = utils::strsplit(string, "\n", true);
            for (auto& str : segments) {
                if (str.empty()) {
                    yPos -= lineHeight;
                    _linesCache.push_back({str, {0, yPos}});
                    continue;
                }

                size_t charIndex = 0;
                while (!str.empty()) {
                    const float lineWidth = renderer.width(str.substr(0, charIndex), textSize);
                    const bool atEnd = charIndex > str.length();
                    const bool isTooWide = lineWidth > maxLineWidth;
                    if (atEnd || isTooWide) {
                        const size_t count = charIndex <= 1 ? charIndex : charIndex - 1;
                        const std::string line = str.substr(0, count);

                        str = str.substr(count);

                        yPos -= lineHeight;
                        _linesCache.push_back({line, {0, yPos}});

                        charIndex = 0;
                    }
                    if (isTooWide) {
                        isWrapped = true;
                    }
                    charIndex++;
                }
            }
        }

        if (!isWrapped && wrapTightly) {
            // Find widest line width
            float widestLineWidth
                = renderer.width(std::max_element(_linesCache.begin(), _linesCache.end(),
                                                  [&](const std::pair<std::string, glm::vec2>& a,
                                                      const std::pair<std::string, glm::vec2>& b) {
                                                      return renderer.width(a.first, textSize)
                                                             < renderer.width(b.first, textSize);
                                                  })
                                     ->first,
                                 textSize);

            if (widestLineWidth <= maxLineWidth) {
                alignment.setWidth(alignment::Pixels(widestLineWidth));
            }
            else {
                alignment.setWidth(defaultWidth);
            }
        }
        else {
            alignment.setWidth(defaultWidth);
        }

        if (wrapTightly) {
            const float height
                = lineHeight * _linesCache.size() - renderer.descenderHeight(textSize);
            alignment.setHeight(alignment::Pixels(height));
        }
        else {
            alignment.setHeight(defaultHeight);
        }

        // Offset the positions
        const glm::vec2 basePos = glm::vec2(0, getPixelSize().y);
        for (auto& line : _linesCache) {
            line.second += basePos;
        }
    }

    // vector<pair<lineText, position>>
    std::vector<std::pair<std::string, glm::vec2>> lines() const
    {
        std::vector<std::pair<std::string, glm::vec2>> l = _linesCache;
        const glm::vec2 pos = getPixelPosition();
        for (auto& line : l) {
            line.second += pos;
        }
        return l;
    }
};

} // namespace fc
