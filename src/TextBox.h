#pragma once
#include "ColoredRect.h"
#include "Text.h"
#include "alignment/MirrorHeight.h"
#include "alignment/MirrorWidth.h"

namespace fc {

class TextBox : public Container {
public:
    Text& text;
    ColoredRect& background;

public:
    TextBox(alignment::ElementAlignment alignment, glm::vec4 backgroundColor, glm::vec4 textColor,
            float textSize, const std::string& text, ShapeRenderer2D& boxRenderer,
            TextRenderer& textRenderer)
        : Container(alignment),
          background(createChild<ColoredRect>(alignment::ElementAlignment(), backgroundColor)),
          text(createChild<Text>(alignment, textColor, textSize, text, textRenderer))
    {
        this->alignment.setX(alignment.x);
        this->alignment.setY(alignment.y);
        this->alignment.setWidth(alignment::MirrorWidth(this->text));
        this->alignment.setHeight(alignment::MirrorHeight(this->text));

        this->text.alignment.setX(alignment::Pixels(0));
        this->text.alignment.setY(alignment::Pixels(0));

        this->text.wrapTightly = true;
    }

    virtual void render(const Window& window, time::Duration delta) override {
        background.render(window, delta);
        text.render(window, delta);
    }
};

} // namespace fc
