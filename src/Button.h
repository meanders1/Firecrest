#pragma once

#include "Color.h"
#include "Container.h"
#include "HorisontalCenterer.h"
#include "RoundedColoredRect.h"
#include "Text.h"
#include "VerticalCenterer.h"

namespace fc {
class Button : public Container {
public:
    using Callback = std::function<void()>;

public:
    Color color;
    Color hoverColor;
    Color clickColor;
    Callback onClickCallback;

    RoundedColoredRect& background;
    Text& text;

public:
    Button(alignment::ElementAlignment alignment, Color color, Color hoverColor, Color clickColor,
           Color textColor, float textSize, const std::string& text, Callback onClickCallback,
           ShapeRenderer2D& shapeRenderer, TextRenderer& textRenderer)
        : Container(alignment),
          color(color),
          hoverColor(hoverColor),
          clickColor(clickColor),
          onClickCallback(onClickCallback),
          background(createChild<RoundedColoredRect>(alignment::ElementAlignment(), color, 15.0f,
                                                     shapeRenderer)),
          text(createChild<VerticalCenterer>().createChild<HorisontalCenterer>().createChild<Text>(
              alignment::ElementAlignment(), textColor, textSize, text, textRenderer))
    {
        focusable = true;
        this->text.wrapTightly = true;
    }

    virtual void onMouseMotionEvent(Input& input, input::MouseMotionEvent event) override
    {
        switch (event.action) {
        case input::MouseMotionAction::Enter:
            background.color = hoverColor;
            break;
        case input::MouseMotionAction::Exit:
            background.color = color;
            break;
        case input::MouseMotionAction::Move:
            break;
        };
    }

    virtual void onMouseButtonEvent(Input& input, input::MouseButtonEvent event) override
    {
        if (event.button == input::MouseButton::Left) {
            switch (event.action) {
            case input::MouseButtonAction::Press:
                background.color = clickColor;
                onClickCallback();
                break;
            case input::MouseButtonAction::Up:
                background.color = hoverColor;
                break;
            }
        }
    }
};
} // namespace fc