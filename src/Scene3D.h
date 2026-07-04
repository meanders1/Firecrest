#pragma once

#include "ColoredRect.h"
#include "Container.h"
#include "FreeCamera.h"
#include "gl/RenderRegion.h"
#include "res/ResourceManager.h"

namespace fc {
class Scene3D : public Container {
public:
    std::vector<res::ModelHandle> models;
    std::unique_ptr<FreeCamera> camera;
    Light light;
    ColoredRect& background;

public:
    Scene3D(alignment::ElementAlignment alignment, ShapeRenderer2D& renderer)
        : Container(alignment),
          camera(std::make_unique<FreeCamera>()),
          background(createChild<ColoredRect>(alignment::ElementAlignment(),
                                              glm::vec4(0.05, 0.05, 0.05, 1)))
    {
        focusable = true;
    }

    void render(const Window& window, time::Duration delta) override {
        // --- Handle input ---
        if (hasFocus() && camera) {
            camera->handleInput(window.getInput(), window, delta);
        }

        // --- Render ---
        Container::render(window, delta);

        gl::RenderRegion::push(getPixelRectangle());
        // Draw
        for (const auto& model : models) {
            if (model) {
                model->render(window, *camera, light);
            }
        }
        gl::RenderRegion::pop();
    }

    virtual void onKeyboardEvent(Input& input, input::KeyboardEvent event) override
    {
        if (event.action == input::KeyAction::Press && event.key == GLFW_KEY_ESCAPE) {
            unFocus();
        }
    }

    virtual void onFocusAquired() override { Element::lockMouse(); }

    virtual void onFocusLost() override { Element::unlockMouse(); }
};
} // namespace fc