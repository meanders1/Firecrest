#pragma once

#include "Scrollable.h"
#include <algorithm>
#include <cctype>

namespace fc {
class TextInput : public Scrollable {
public:
    Text& text;

    Color backgroundColor;
    Color selectionColor = Color(0.00f, 0.46f, 0.82f, 0.8f);

private:
    int32_t _cursorPosition = 0;
    int32_t _selectionAnchor = 0;

    int _cursorBlinkCounter = 0;
    bool _showCursor = false;
    ShapeRenderer2D& _renderer;

public:
    TextInput(alignment::ElementAlignment alignment, Color backgroundColor, Color textColor,
              float textSize, const std::string& text, ShapeRenderer2D& boxRenderer,
              TextRenderer& textRenderer)
        : Scrollable(alignment, boxRenderer),
          text(createChild<Text>(alignment, textColor, textSize, text, textRenderer)),
          _renderer(boxRenderer),
          backgroundColor(backgroundColor)
    {
        focusable = true;
        this->text.wrapTightly = true;
    }

    int32_t selectionStart() const { return std::min(_selectionAnchor, _cursorPosition); }
    int32_t selectionEnd() const { return std::max(_selectionAnchor, _cursorPosition); }
    bool hasSelection() const { return selectionStart() != selectionEnd(); }
    std::string selectedText() const
    {
        return text.text.substr(selectionStart(), selectionEnd() - selectionStart());
    }
    void selectAll()
    {
        _selectionAnchor = 0;
        _cursorPosition = static_cast<int32_t>(text.text.size());
    }

    virtual void onLetterTyped(Input& input, input::UnicodeCodePoint letter) override
    {
        if (static_cast<char>(letter) < 0)
            return;

        replaceSelection(std::string(1, static_cast<char>(letter)));
    }

    virtual void render(const Window& window, time::Duration delta) override
    {
        const glm::vec2 size = getPixelSize();
        const glm::vec2 pos = getPixelPosition();
        _renderer.rect(window, pos, size, backgroundColor);

        renderSelection(window);
        Scrollable::render(window, delta);

        // Draw cursor
        const int interval = 60; // Blink every 30 frames
        if (_cursorBlinkCounter % interval < interval / 2 && _showCursor) {
            const auto pos = cursorPixelPos();
            const float lineHeight = text.renderer.lineHeight(text.textSize);
            const float relY = -0.2f * lineHeight;
            _renderer.rect(window, glm::vec3(pos, 0) + glm::vec3(0, relY - getScrollOffset(), 0),
                           {1.5, lineHeight}, text.color);
        }
        _cursorBlinkCounter++;
    }

    virtual void onKeyboardEvent(Input& input, input::KeyboardEvent event) override
    {
        if (event.action == input::KeyAction::Press || event.action == input::KeyAction::Repeat) {
            _cursorBlinkCounter = 0;

            switch (event.key) {
            case GLFW_KEY_BACKSPACE:
                if (hasSelection()) {
                    replaceSelection("");
                }
                else if (_cursorPosition > 0) {
                    _selectionAnchor = _cursorPosition - 1;
                    replaceSelection("");
                }
                break;

            case GLFW_KEY_DELETE:
                if (hasSelection()) {
                    replaceSelection("");
                }
                else if (_cursorPosition < text.text.size()) {
                    _selectionAnchor = _cursorPosition + 1;
                    replaceSelection("");
                }
                break;

            case GLFW_KEY_ENTER: {
                replaceSelection("\n");
                break;
            }

            case GLFW_KEY_LEFT: {
                const bool shift_pressed = input.keyPressed(GLFW_KEY_LEFT_SHIFT)
                                           || input.keyPressed(GLFW_KEY_RIGHT_SHIFT);
                if (!shift_pressed && hasSelection()) {
                    _cursorPosition = selectionStart();
                    updateSelection(false);
                    break;
                }
                if (input.keyPressed(GLFW_KEY_LEFT_CONTROL)
                    || input.keyPressed(GLFW_KEY_RIGHT_CONTROL)) {

                    const std::string& s = text.text;

                    if (_cursorPosition > 0) {
                        // Step 2: skip non-whitespace (the word)
                        while (
                            _cursorPosition > 0
                            && !std::isspace(static_cast<unsigned char>(s[_cursorPosition - 1]))) {
                            --_cursorPosition;
                        }

                        // Step 1: skip whitespace to the left
                        while (
                            _cursorPosition > 0
                            && std::isspace(static_cast<unsigned char>(s[_cursorPosition - 1]))) {
                            --_cursorPosition;
                        }
                    }
                }
                else {
                    --_cursorPosition;
                }

                clampCursor();
                updateSelection(shift_pressed);
                break;
            }

            case GLFW_KEY_RIGHT: {
                const bool shift_pressed = input.keyPressed(GLFW_KEY_LEFT_SHIFT)
                                           || input.keyPressed(GLFW_KEY_RIGHT_SHIFT);
                if (!shift_pressed && hasSelection()) {
                    _cursorPosition = selectionEnd();
                    updateSelection(false);
                    break;
                }
                if (input.keyPressed(GLFW_KEY_LEFT_CONTROL)
                    || input.keyPressed(GLFW_KEY_RIGHT_CONTROL)) {

                    const std::string& s = text.text;
                    const size_t len = s.size();

                    // Step 1: skip non-whitespace (current word)
                    while (_cursorPosition < len
                           && !std::isspace(static_cast<unsigned char>(s[_cursorPosition]))) {
                        ++_cursorPosition;
                    }

                    // Step 2: skip whitespace
                    while (_cursorPosition < len
                           && std::isspace(static_cast<unsigned char>(s[_cursorPosition]))) {
                        ++_cursorPosition;
                    }
                }
                else {
                    ++_cursorPosition;
                }

                clampCursor();
                updateSelection(shift_pressed);
                break;
            }

            case GLFW_KEY_A:
                if (input.keyPressed(GLFW_KEY_LEFT_CONTROL)
                    || input.keyPressed(GLFW_KEY_RIGHT_CONTROL)) {
                    selectAll();
                }
                break;

            case GLFW_KEY_C:
                if ((input.keyPressed(GLFW_KEY_LEFT_CONTROL)
                     || input.keyPressed(GLFW_KEY_RIGHT_CONTROL))
                    && hasSelection()) {
                    input.setClipboard(selectedText());
                }
                break;

            case GLFW_KEY_X:
                if ((input.keyPressed(GLFW_KEY_LEFT_CONTROL)
                     || input.keyPressed(GLFW_KEY_RIGHT_CONTROL))
                    && hasSelection()) {
                    input.setClipboard(selectedText());
                    replaceSelection("");
                }
                break;

            case GLFW_KEY_UP: {
                const bool shift = input.keyPressed(GLFW_KEY_LEFT_SHIFT)
                                   || input.keyPressed(GLFW_KEY_RIGHT_SHIFT);
                const uint32_t currentLine = cursorLineNumber();
                // First line
                if (currentLine == 0) {
                    _cursorPosition = 0;
                    updateSelection(shift);
                    break;
                }

                const uint32_t line = currentLine - 1;

                const uint32_t characterOnLine = cursorPosOnLine();
                const uint32_t lineChars = text.lines()[line].first.size();

                uint32_t charcount = 0;
                for (int i = 0; i < line; i++) {
                    charcount += text.lines()[i].first.size();
                }

                if (lineChars >= characterOnLine) {
                    _cursorPosition = charcount + characterOnLine;
                }
                else {
                    _cursorPosition = charcount + lineChars - 1;
                }

                updateSelection(shift);
                break;
            }

            case GLFW_KEY_DOWN: {
                const bool shift = input.keyPressed(GLFW_KEY_LEFT_SHIFT)
                                   || input.keyPressed(GLFW_KEY_RIGHT_SHIFT);
                const uint32_t line = cursorLineNumber();
                if (line >= text.lines().size() - 1) {
                    _cursorPosition = text.text.size();
                    clampCursor();
                    updateSelection(shift);
                    break;
                }

                const uint32_t charIndex = cursorPosOnLine();
                _cursorPosition += text.lines()[line].first.length() - charIndex;

                const std::string lineUnder = text.lines()[line + 1].first;
                if (lineUnder.length() - 1 >= charIndex) {
                    _cursorPosition += charIndex;
                }
                else {
                    _cursorPosition += lineUnder.size();
                }
                clampCursor();
                updateSelection(shift);
                break;
            }

            case GLFW_KEY_V: {
                const char* clipboard = input.clipboard();
                if (input.keyPressed(GLFW_KEY_LEFT_CONTROL)
                    || input.keyPressed(GLFW_KEY_RIGHT_CONTROL)) {
                    replaceSelection(clipboard);
                }
                break;
            }

            default:
                break;
            }
        }
    }

    virtual void onFocusAquired() override
    {
        _showCursor = true;
        _cursorBlinkCounter = 0;
    }
    virtual void onFocusLost() override { _showCursor = false; }

    // Get the line the cursor is currently on
    uint32_t cursorLineNumber() const
    {
        const auto& lines = text.lines();
        if (lines.size() <= 1)
            return 0;

        uint32_t characterIndex = 0;
        for (size_t lineNum = 0; lineNum < lines.size(); lineNum++) {
            const std::string& line = lines[lineNum].first;
            characterIndex += line.length();

            if (characterIndex > _cursorPosition) {
                return lineNum;
            }
            if (characterIndex == _cursorPosition) {
                return lineNum;
            }
        }
        return lines.size() - 1;
    }

    uint32_t cursorPosOnLine() const
    {
        auto line = cursorLineNumber();
        uint32_t numChars = 0;
        for (uint32_t i = 0; i < line; i++) {
            numChars += text.lines()[i].first.length();
        }
        return _cursorPosition - numChars;
    }

    // Get the position of the cursor on screen
    glm::vec2 cursorPixelPos() const
    {
        uint32_t line = cursorLineNumber();
        uint32_t col = cursorPosOnLine();

        const auto lineText = text.lines()[line].first.substr(0, col);

        const glm::vec2 pos = getPixelPosition();

        // Calculate X: width of text before cursor on this line
        const float x = pos.x + text.renderer.width(lineText, text.textSize);

        // Calculate Y: top + line * lineHeight
        const float lineHeight = text.renderer.lineHeight(text.textSize);
        const float y = pos.y + getPixelSize().y - (line + 1) * lineHeight;

        return glm::vec2(x, y);
    }

private:
    void updateSelection(bool extending)
    {
        if (!extending)
            _selectionAnchor = _cursorPosition;
    }

    void replaceSelection(const std::string& replacement)
    {
        const int32_t start = selectionStart();
        const int32_t end = selectionEnd();
        text.text.replace(start, end - start, replacement);
        _cursorPosition = start + static_cast<int32_t>(replacement.size());
        _selectionAnchor = _cursorPosition;
        clampCursor();
    }

    void renderSelection(const Window& window)
    {
        if (!hasSelection())
            return;

        const auto lines = text.lines();
        const float lineHeight = text.renderer.lineHeight(text.textSize);
        int32_t lineStart = 0;

        gl::RenderRegion::push(getPixelRectangle(), gl::RenderRegion::Mode::Scissor);

        for (const auto& line : lines) {
            const int32_t lineEnd = lineStart + static_cast<int32_t>(line.first.size());
            const int32_t spanStart = std::max(selectionStart(), lineStart);
            const int32_t spanEnd = std::min(selectionEnd(), lineEnd);

            if (spanStart < spanEnd) {
                const auto before = line.first.substr(0, spanStart - lineStart);
                const auto selected = line.first.substr(spanStart - lineStart, spanEnd - spanStart);
                const float x = getPixelPosition().x + text.renderer.width(before, text.textSize);
                const float width = text.renderer.width(selected, text.textSize);
                const float y = line.second.y - getScrollOffset() - 0.2f * lineHeight;
                _renderer.rect(window, {x, y}, {std::max(width, 3.0f), lineHeight}, selectionColor);
            }

            lineStart = lineEnd;
        }

        gl::RenderRegion::pop();
    }

    void clampCursor()
    {
        if (_cursorPosition < 0) {
            _cursorPosition = 0;
        }
        else if (_cursorPosition > text.text.length()) {
            _cursorPosition = text.text.length();
        }
    }
};
} // namespace fc