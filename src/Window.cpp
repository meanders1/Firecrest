#include "Window.h"
#include "gl/OpenGL.h"
#include "gl/RenderRegion.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

namespace fc {

void APIENTRY debugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                          GLsizei length, const char* message, const void* userParam);

Window::Window(WindowProperties& properties)
{
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (properties.antialiasing)
        glfwWindowHint(GLFW_SAMPLES, 4);

    if (!properties.resizable)
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif // _DEBUG

    glfwWindowHint(GLFW_VISIBLE,
                   GLFW_FALSE); // Do not show the window until configuration is done
    _handle = glfwCreateWindow(properties.width, properties.height, properties.title.c_str(), NULL,
                               NULL);
    if (!_handle) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
    }
    _input.setWindow(_handle);

    glfwSetWindowUserPointer(_handle, this);
    glfwMakeContextCurrent(_handle);

    if (properties.vsync)
        glfwSwapInterval(1);

    if (properties.maximized)
        glfwMaximizeWindow(_handle);

    glfwSetCharCallback(_handle, [](GLFWwindow* window, unsigned int codepoint) {
        Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
        windowObj->_input.characterCallback(window, codepoint);
    });

    glfwSetKeyCallback(
        _handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
            windowObj->_input.keyCallback(window, key, scancode, action, mods);
        });

    glfwSetCursorPosCallback(_handle, [](GLFWwindow* window, double xpos, double ypos) {
        Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
        windowObj->_input.mousePosCallback(window, xpos, ypos);
    });

    glfwSetMouseButtonCallback(_handle, [](GLFWwindow* window, int button, int action, int mods) {
        Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
        windowObj->_input.mouseButtonCallback(window, button, action, mods);
    });

    glfwSetScrollCallback(_handle, [](GLFWwindow* window, double xoffset, double yoffset) {
        Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
        windowObj->_input.scrollCallback(window, xoffset, yoffset);
    });

    glfwShowWindow(_handle);

    if (!gladLoadGL()) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return;
    }

    glfwSetWindowSizeCallback(_handle, sizeCallback);
    glfwSetWindowIconifyCallback(_handle, iconifyCallback);
    glfwSetWindowMaximizeCallback(_handle, maximizeCallback);

    if (properties.antialiasing) {
        glEnable(GL_MULTISAMPLE);
    }
#ifdef _DEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugOutput, NULL);
    }
#endif // _DEBUG

    resized();
    glClearColor(0, 0, 0, 1);
}

Window::~Window()
{
    glfwDestroyWindow(_handle);
}

void Window::display()
{
    _input.update();
    glfwSwapBuffers(_handle);
    glfwPollEvents();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(_handle);
}

void Window::setTitle(const std::string& title)
{
    glfwSetWindowTitle(_handle, title.c_str());
}

void Window::lockMouse()
{
    glfwSetInputMode(_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::freeMouse()
{
    glfwSetInputMode(_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Window::isMouseLocked() const
{
    return glfwGetInputMode(_handle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

bool Window::isMouseFree() const
{
    return glfwGetInputMode(_handle, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void Window::clearScreen() const
{
    glClearColor(_clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::clearColor(const Color& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    _clearColor = color;
}

int Window::width() const
{
    int width, height;
    glfwGetWindowSize(_handle, &width, &height);
    return width;
}

int Window::height() const
{
    int width, height;
    glfwGetWindowSize(_handle, &width, &height);
    return height;
}

glm::ivec2 Window::dimensions() const
{
    int width, height;
    glfwGetWindowSize(_handle, &width, &height);
    return {width, height};
}

glm::mat4 Window::orthographicProjection() const
{
    glm::ivec2 dims = dimensions();
    return glm::ortho(0.0f, static_cast<float>(dims.x), 0.0f, static_cast<float>(dims.y), -40.0f,
                      40.0f);
}

void Window::resized()
{
    glm::ivec2 dim = dimensions();
    gl::RenderRegion::base = {0.0f, 0.0f, static_cast<float>(dim.x), static_cast<float>(dim.y)};
    gl::RenderRegion::applyBase();
}

void Window::sizeCallback(GLFWwindow* window, int width, int height)
{
    Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    windowObj->sizeCallback(width, height);
}

void Window::sizeCallback(int width, int height)
{
    resized();
}

void Window::iconifyCallback(GLFWwindow* window, int iconified)
{
    Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    windowObj->iconifyCallback(iconified);
}

void Window::iconifyCallback(int iconified)
{
    resized();
}

void Window::maximizeCallback(GLFWwindow* window, int maximized)
{
    Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
    windowObj->maximizeCallback(maximized);
}

void Window::maximizeCallback(int maximized)
{
    resized();
}

void APIENTRY debugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                          GLsizei length, const char* message, const void* userParam)
{

    // ignore non-significant error/warning codes
    // if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << std::endl;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << std::endl;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;

    std::cout << std::endl;
}

} // namespace fc