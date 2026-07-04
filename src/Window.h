#pragma once

#include "Color.h"
#include "Input.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>

namespace fc {

struct WindowProperties {
    int width, height;
    bool vsync;
    std::string title;
    bool resizable;
    bool iconified;
    bool maximized;
    bool antialiasing;
    WindowProperties()
        : width(100),
          height(100),
          vsync(true),
          title("Title"),
          resizable(true),
          iconified(false),
          maximized(false),
          antialiasing(false)
    {
    }
};

class Window {
private:
    GLFWwindow* _handle;
    Input _input;
    glm::vec4 _clearColor;

public:
    Window(WindowProperties& properties);
    ~Window();

    Window(const Window& window) = delete;
    Window& operator=(const Window& window) = delete;

    void display();
    bool shouldClose() const;
    void setTitle(const std::string& title);

    void lockMouse();
    void freeMouse();
    bool isMouseLocked() const;
    bool isMouseFree() const;

    void clearColor(const Color& color);
    void clearScreen() const;

    inline const GLFWwindow* const handle() const { return _handle; }
    inline GLFWwindow* handle() { return _handle; }
    inline const Input& getInput() const { return _input; }
    inline Input& getInput() { return _input; }
    int width() const;
    int height() const;
    glm::ivec2 dimensions() const;
    glm::mat4 orthographicProjection() const;

private:
    void resized();

    static void sizeCallback(GLFWwindow* window, int width, int height);
    void sizeCallback(int width, int height);

    static void iconifyCallback(GLFWwindow* window, int iconified);
    void iconifyCallback(int iconified);

    static void maximizeCallback(GLFWwindow* window, int maximized);
    void maximizeCallback(int maximized);
};

} // namespace fc