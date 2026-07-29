#pragma once

#include "Shader.h"
#include "Shape.h"

namespace fc::gl {

template <typename tVertex>
class Painter {
public:
    using DrawCallback = std::function<void(const Window&, const Shape<tVertex>&, const Shader&)>;

public:
    Shader shader;
    DrawCallback preDrawCallback;

    /// When the predraw callback is called, the vertex buffer, index buffer and shader is already
    /// bound.
    Painter(Shader&& shader_, DrawCallback preDrawCallback_)
        : shader(std::move(shader_)), preDrawCallback(std::move(preDrawCallback_))
    {
    }

    Painter(Painter&&) noexcept = default;
    Painter& operator=(Painter&&) noexcept = default;

    Painter(const Painter&) = delete;
    Painter& operator=(const Painter&) = delete;

    void draw(const Window& window, const Shape<tVertex>& shape)
    {
        shader.bind();
        shape.vao.bind();
        shape.ibo.bind();

        preDrawCallback(window, shape, shader);
        glDrawElements(shape.drawMode, shape.ibo.getCount(), GL_UNSIGNED_INT, nullptr);

        shape.ibo.unbind();
        shape.vao.unbind();
        shader.unbind();
    }
};

} // namespace fc::gl